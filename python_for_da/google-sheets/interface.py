import random
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from faker import Faker
from scipy import stats

import gspread
from gspread.utils import rowcol_to_a1, ExportFormat

def create_dataset(row_count=200, savepath="data.csv"):
    """
    решил использовать прикольные распределения чисел, раз нельзя нейронкой генерить... среди них:

    дискретные
    zipf - не знаю что конкретно моделирует, но обычно сэмплирует маленькие числа и рандомно может насэмплировать огромное
    benford - вероятность встретить первую цифру в случайном числе из реальных данных (эмпирический закон)
    borel - клетка порождает клетки по распределению Пуассона и больше не может порождать клетки,
            затем порожденные клетки делают то же самое до тех пор пока не останутся только неспособные к порождению клетки

    непрерывные (чуть менее интересные)
    cauchy - выбросы (из-за этого выборочное среднее не сходится, т.к. мат.ожидания не существует)
    arcsin - скучное распределение на [0, 1], но прикольно что вероятность 0 и 1 выше чем 0.5
    levy - тоже колоссальные выбросы из-за чего E[X]=inf
    """

    fake = Faker("ru_RU")

    def benford(size=200):
        digits = np.arange(1, 10)
        probs = np.log10(1 + 1 / digits)
        rng = np.random.default_rng()
        return rng.choice(digits, size=size, p=probs)

    def borel_sample(lam):
        rng = np.random.default_rng()
        total = 1
        active = 1
        while active:
            active = rng.poisson(lam * active)
            total += active
        return total

    df = pd.DataFrame(data={
        "int_column_1": pd.Series(np.random.zipf(1.5, row_count), dtype=np.int32),
        "int_column_2": pd.Series(benford(row_count), dtype=np.int32),
        "int_column_3": pd.Series([borel_sample(0.6) for _ in range(row_count)], dtype=np.int32),
        "float_column_1": pd.Series(stats.cauchy.rvs(size=row_count), dtype=np.float32),
        "float_column_2": pd.Series(stats.arcsine.rvs(size=row_count), dtype=np.float32),
        "float_column_3": pd.Series(stats.levy.rvs(size=row_count), dtype=np.float32),
        "string_column_1": pd.Series([fake.name() for _ in range(row_count)]),
        "string_column_2": pd.Series([fake.company() for _ in range(row_count)]),
        "string_column_3": pd.Series([fake.address() for _ in range(row_count)]),
        "string_column_4": pd.Series([fake.text() for _ in range(row_count)]),
    })

    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    fig.suptitle("Сводка по сгенерированным числовым значениям", fontsize=30)
    for column_name, ax in zip(df.columns[:6], axes.flatten()):
        ax.plot(df[column_name])
        ax.set_title(column_name)
        ax.set_xticks([])
    plt.tight_layout()
    plt.show()

    print("И еще немного из строковых нагенеренных библиотекой faker")
    for column_name in df.columns[6:]:
        print(f"\n{column_name}\n")
        print("\n".join(df[column_name][:3]))

    df = df.sample(frac=1, axis=1) # перемешать колонки
    df.to_csv(savepath, index=False, encoding="utf-8")
    return df


def init_sheet():
    global gc, sh
    gc = gspread.oauth(
        credentials_filename="creds.json",
        authorized_user_filename="user.json",
    )
    try:
        id = gc.open("homework_python_for_da").id
        gc.del_spreadsheet(id)
    except gspread.exceptions.SpreadsheetNotFound:
        pass
    sh = gc.create("homework_python_for_da")
    return sh.url


def fill_sheet(df: pd.DataFrame):
    global ws
    row_count, column_сount = df.shape

    ws = sh.get_worksheet(0)
    ws.update_title("Dataset 1")
    ws.delete_rows(row_count + 1, ws.row_count)
    ws.delete_columns(column_сount + 1, ws.column_count)
    ws.update([df.columns.values.tolist()] + df.values.tolist())

    ws2 = sh.add_worksheet("Copy", row_count + 1, column_сount) # + 1 для названий колонок
    ws2.update([df.columns.values.tolist()] + df.values.tolist())


def column_sum():
    ws.add_cols(1)
    col_count = ws.col_count
    row_count = ws.row_count
    ws.update(
        [["SUM"]] + [[f"=SUM({rowcol_to_a1(i, 1)}:{rowcol_to_a1(i, col_count - 1)})"] for i in range(2, row_count + 1)],
        rowcol_to_a1(1, col_count),
        value_input_option="USER_ENTERED",
    )


def save_column_sum(savepath="column_sum.csv"):
    row_count = ws.row_count
    col_count = ws.col_count
    data = ws.get(f"{rowcol_to_a1(1, col_count)}:{rowcol_to_a1(row_count, col_count)}")
    df = pd.DataFrame(data[1:], columns=data[0])
    df.to_csv(savepath, index=False)


def calculate_mean():
    ws.add_cols(1)
    row_count = ws.row_count
    col_count = ws.col_count

    data = ws.get_all_values(value_render_option="UNFORMATTED_VALUE")
    numbers = [
        x for row in data for x in row
        if type(x) in (int, float)
    ]
    what_to_save = sum(numbers) / len(numbers)

    # либо можно просто вставить формулу, я проверил одно и то же
    # решил оставить подсчет в питоне, потому что вставить формулу уже было
    # what_to_save = f"=AVERAGE({rowcol_to_a1(2, 1)}:{rowcol_to_a1(row_count, col_count-1)})"

    ws.update(
        [[what_to_save]],
        rowcol_to_a1(1, col_count),
        value_input_option="USER_ENTERED",
    )


def rename_column(num, new_name):    
    ws.update(
        [[new_name]],
        rowcol_to_a1(1, num),
        value_input_option="USER_ENTERED",
    )


def delete_random_cells(p=0.1):
    row_count = ws.row_count
    col_count = ws.col_count

    cells = [
        rowcol_to_a1(r, c)
        for r in range(1, row_count + 1)
        for c in range(1, col_count + 1)
        if random.random() < p
    ]

    if cells:
        ws.batch_clear(cells)


def download_worksheet(savepath="processed.csv"):
    data = sh.export(format=ExportFormat.CSV)
    with open(savepath, "wb") as f:
        f.write(data)
