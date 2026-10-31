"""SAM ViT-B (quantized ONNX), CPU, без torch и segment_anything.

Установка в окружение проекта:
    uv pip install onnxruntime

Из Python / ноутбука (сам ноутбук менять не требуется):
    from sam_inference_vibecoded import segment_cat
    mask = segment_cat("images.jpeg")  # (H, W), uint8, фон=0, объект=255
    mask = segment_cat("images.jpeg", box=(20, 10, 220, 200))

Из терминала:
    python sam_inference_vibecoded.py images.jpeg -o cat_mask.png
    python sam_inference_vibecoded.py images.jpeg --point 120 80

Первый вызов скачивает ~75 MB готовых весов; дальше работает офлайн.
SAM не распознаёт класс «кот»: без подсказок выбирается объект в центре.
Если центр попал в фон/деталь кота, задайте рамку --box X1 Y1 X2 Y2
или точку --point X Y на коте (координаты исходного изображения).
Для нескольких фото используйте один SamOnnx, чтобы не загружать веса заново.

Веса: https://huggingface.co/vietanhdev/segment-anything-onnx-models
Экспорт: https://github.com/vietanhdev/samexporter
"""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import sys
import tempfile
import urllib.request
import zipfile

import cv2
import numpy as np


MODEL_URL = (
    "https://huggingface.co/vietanhdev/segment-anything-onnx-models/resolve/"
    "9effc01a9e135621d710d49159f1ffb0b6f724dc/sam_vit_b_01ec64_quant.zip"
)
MODEL_NAMES = (
    "sam_vit_b_01ec64.encoder.quant.onnx",
    "sam_vit_b_01ec64.decoder.quant.onnx",
)
DEFAULT_MODEL_DIR = Path(__file__).resolve().parent / ".sam_onnx"


def _model_paths(model_dir: Path) -> tuple[Path, Path]:
    paths = tuple(model_dir / name for name in MODEL_NAMES)
    if all(path.is_file() for path in paths):
        return paths
    model_dir.mkdir(parents=True, exist_ok=True)
    print("Скачиваю SAM ViT-B ONNX (~75 MB)...", file=sys.stderr)
    # Временные файлы удаляются и после сетевой ошибки. Готовые веса
    # появляются в кеше только после полной загрузки и проверки ZIP CRC.
    with tempfile.TemporaryDirectory(dir=model_dir) as temp:
        archive = Path(temp) / "models.zip"
        with urllib.request.urlopen(MODEL_URL, timeout=120) as response:
            with archive.open("wb") as output:
                shutil.copyfileobj(response, output)
        with zipfile.ZipFile(archive) as bundle:
            for name in MODEL_NAMES:
                with bundle.open(name) as source, (Path(temp) / name).open("wb") as target:
                    shutil.copyfileobj(source, target)
        for name, path in zip(MODEL_NAMES, paths):
            (Path(temp) / name).replace(path)
    return paths


class SamOnnx:
    """Один encoder/decoder на несколько изображений; инференс на CPU."""

    def __init__(self, model_dir: str | Path = DEFAULT_MODEL_DIR):
        try:
            import onnxruntime as ort
        except ImportError as exc:
            raise ImportError("Установите ONNX Runtime: uv pip install onnxruntime") from exc
        encoder, decoder = _model_paths(Path(model_dir).expanduser())
        self.encoder = ort.InferenceSession(str(encoder), providers=["CPUExecutionProvider"])
        self.decoder = ort.InferenceSession(str(decoder), providers=["CPUExecutionProvider"])

    def segment(
        self,
        image_path: str | Path,
        *,
        point: tuple[float, float] | None = None,
        box: tuple[float, float, float, float] | None = None,
    ) -> np.ndarray:
        """Вернуть бинарную маску uint8 исходного размера (0 или 255).

        point — положительная точка (x, y); box — (x1, y1, x2, y2).
        Можно передать обе подсказки. Без них используется центр фото.
        """
        path = Path(image_path).expanduser()
        if not path.is_file():
            raise FileNotFoundError(f"Изображение не найдено: {path}")
        image = cv2.imdecode(np.fromfile(path, dtype=np.uint8), cv2.IMREAD_COLOR)
        if image is None:
            raise ValueError(f"Не удалось прочитать изображение: {path}")
        h, w = image.shape[:2]
        coords, labels = [], []
        if point is None and box is None:
            point = (w / 2, h / 2)
        if point is not None:
            x, y = point
            if not (0 <= x < w and 0 <= y < h):
                raise ValueError(f"Точка должна лежать внутри фото {w}x{h}")
            coords.append((x, y))
            labels.append(1)
        if box is not None:
            x1, y1, x2, y2 = box
            if not (0 <= x1 < x2 <= w and 0 <= y1 < y2 <= h):
                raise ValueError(f"Нужна рамка 0 <= x1 < x2 <= {w}, 0 <= y1 < y2 <= {h}")
            coords.extend([(x1, y1), (x2, y2)])
            labels.extend([2, 3])
        else:
            coords.append((0, 0))
            labels.append(-1)  # Padding prompt для запроса без рамки.

        # ResizeLongestSide из SAM: округление, сохранение пропорций.
        size = 1024
        scale = size / max(h, w)
        nh, nw = int(h * scale + 0.5), int(w * scale + 0.5)
        rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        resized = cv2.resize(rgb, (nw, nh), interpolation=cv2.INTER_LINEAR)
        # В этом экспорте preprocessing встроен в HWC encoder. Поддержан
        # также экспорт без preprocessing (NCHW), с теми же весами SAM.
        encoder_input = self.encoder.get_inputs()[0]
        if len(encoder_input.shape) == 3:
            tensor = resized.astype(np.float32)
        else:
            normalized = (resized.astype(np.float32) - np.array(
                [123.675, 116.28, 103.53], dtype=np.float32
            )) / np.array([58.395, 57.12, 57.375], dtype=np.float32)
            padded = np.zeros((size, size, 3), dtype=np.float32)
            padded[:nh, :nw] = normalized
            tensor = padded.transpose(2, 0, 1)[None]
        embeddings = self.encoder.run(None, {encoder_input.name: tensor})[0]
        points = np.asarray(coords, dtype=np.float32)
        points *= np.array([nw / w, nh / h], dtype=np.float32)
        masks, scores = self.decoder.run(["low_res_masks", "iou_predictions"], {
            "image_embeddings": embeddings,
            "point_coords": points[None],
            "point_labels": np.asarray([labels], dtype=np.float32),
            "mask_input": np.zeros((1, 1, 256, 256), dtype=np.float32),
            "has_mask_input": np.zeros(1, dtype=np.float32),
            "orig_im_size": np.asarray([h, w], dtype=np.float32),
        })
        # При multimask экспорт возвращает специальную single-mask
        # гипотезу первой, затем три альтернативы для неоднозначной точки.
        start = 1 if masks.shape[1] == 4 and box is None else 0
        best = start + int(np.argmax(scores[0, start:]))
        # У готового decoder выход masks обрезан до фиксированных 683x1024
        # при экспорте. Поэтому восстанавливаем размер из low_res_masks
        # сами: 256 → 1024, убрать padding, вернуть исходные H/W.
        # Порог применяется ПОСЛЕ интерполяции логитов.
        square = cv2.resize(masks[0, best], (size, size), interpolation=cv2.INTER_LINEAR)
        logits = cv2.resize(square[:nh, :nw], (w, h), interpolation=cv2.INTER_LINEAR)
        return (logits > 0).astype(np.uint8) * 255


def segment_cat(
    image_path: str | Path,
    *,
    point: tuple[float, float] | None = None,
    box: tuple[float, float, float, float] | None = None,
    model_dir: str | Path = DEFAULT_MODEL_DIR,
) -> np.ndarray:
    """Путь к фото → маска; для серии фото удобнее SamOnnx.segment()."""
    return SamOnnx(model_dir).segment(image_path, point=point, box=box)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("image", type=Path)
    parser.add_argument("-o", "--output", type=Path, help="Путь для PNG-маски")
    parser.add_argument("--point", nargs=2, type=float, metavar=("X", "Y"))
    parser.add_argument("--box", nargs=4, type=float, metavar=("X1", "Y1", "X2", "Y2"))
    parser.add_argument("--model-dir", type=Path, default=DEFAULT_MODEL_DIR)
    args = parser.parse_args()
    try:
        mask = segment_cat(args.image, point=args.point, box=args.box, model_dir=args.model_dir)
        output = args.output or args.image.with_name(args.image.stem + "_sam_mask.png")
        if output.suffix.lower() != ".png":
            raise ValueError("Для бинарной маски укажите выходной файл .png")
        if output.resolve() == args.image.resolve():
            raise ValueError("Выходной файл должен отличаться от исходного фото")
        ok, encoded = cv2.imencode(".png", mask)
        if not ok:
            raise RuntimeError("Не удалось закодировать маску в PNG")
        output.parent.mkdir(parents=True, exist_ok=True)
        encoded.tofile(output)
    except (OSError, ValueError, ImportError, RuntimeError, zipfile.BadZipFile) as exc:
        parser.exit(1, f"Ошибка: {exc}\n")
    print(output)


if __name__ == "__main__":
    main()
