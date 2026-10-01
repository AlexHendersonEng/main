from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
import csv
import numpy as np
from scipy.io import loadmat


@dataclass(slots=True)
class SimulationResult:
    data: dict[str, np.ndarray]
    descriptions: dict[str, str] | None = None
    time_variable: str = "time"

    def __post_init__(self) -> None:
        self.data = {k: np.asarray(v).reshape(-1) for k, v in self.data.items()}
        self.descriptions = self.descriptions or {}

        # Fall back to the first variable if no explicit time variable exists.
        if self.time_variable not in self.data and self.data:
            self.time_variable = next(iter(self.data))

    def variable_names(self) -> list[str]:
        return list(self.data.keys())

    def has_variable(self, name: str) -> bool:
        return name in self.data

    def get_data(self, name: str) -> np.ndarray:
        try:
            return self.data[name]
        except KeyError as exc:
            available = ", ".join(self.variable_names())
            raise KeyError(
                f"Variable '{name}' not found. Available: {available}"
            ) from exc

    def get_description(self, name: str) -> str | None:
        return self.descriptions.get(name)

    @property
    def time(self) -> np.ndarray:
        return self.get_data(self.time_variable)


def _decode_char_matrix(matrix: np.ndarray) -> list[str]:
    arr = np.asarray(matrix)

    if arr.ndim == 1:
        arr = arr.reshape(1, -1)

    if arr.dtype.kind in {"U", "S"}:
        rows = ["".join(map(str, row.tolist())) for row in arr]
    else:
        rows = ["".join(chr(int(v)) for v in row if int(v) != 0) for row in arr]

    return [row.rstrip(" \x00") for row in rows]


def _require_key(content: dict[str, np.ndarray], key: str) -> np.ndarray:
    if key not in content:
        raise ValueError(f"Missing '{key}' in MAT file.")
    return np.asarray(content[key])


def load_mat(file_path: str | Path) -> SimulationResult:
    # Load mat file
    mat = loadmat(str(file_path), chars_as_strings=False, spmatrix=False)

    # Validate mat file structure
    aclass = _require_key(mat, "Aclass")
    aclass_rows = _decode_char_matrix(aclass)
    if len(aclass_rows) < 4:
        raise ValueError("Invalid Aclass entry in MAT file.")

    storage = aclass_rows[3].strip().lower()
    transpose_all = storage == "bintrans"

    name = _require_key(mat, "name")
    description = _require_key(mat, "description")
    data_info = _require_key(mat, "dataInfo")
    data_1 = _require_key(mat, "data_1")
    data_2 = _require_key(mat, "data_2")

    # Process contents
    if transpose_all:
        # SciPy already materializes numeric matrices in the expected orientation.
        # For OpenModelica MATv4 binTrans files, metadata tables still need transposition.
        name = name.T
        description = description.T
        data_info = data_info.T

    names = _decode_char_matrix(name)
    descriptions = _decode_char_matrix(description)

    if data_info.ndim != 2 or data_info.shape[1] < 2:
        raise ValueError("Invalid dataInfo shape in MAT file.")

    data_1 = np.atleast_2d(np.asarray(data_1, dtype=float))
    data_2 = np.atleast_2d(np.asarray(data_2, dtype=float))

    result_data: dict[str, np.ndarray] = {}
    result_desc: dict[str, str] = {}

    time_from_table = (
        data_2[0, :].reshape(-1) if data_2.size else np.array([], dtype=float)
    )
    detected_time_name = "time"

    for i, variable_name in enumerate(names):
        if not variable_name:
            continue
        if i >= data_info.shape[0]:
            break

        table_selector = int(data_info[i, 0])
        index = int(data_info[i, 1])

        if table_selector == 0:
            values = time_from_table
            detected_time_name = variable_name
        elif table_selector == 1:
            row = abs(index) - 1
            if row < 0 or row >= data_1.shape[0]:
                continue
            values = data_1[row, :].reshape(-1)
        elif table_selector == 2:
            row = abs(index) - 1
            if row < 0 or row >= data_2.shape[0]:
                continue
            values = data_2[row, :].reshape(-1)
        else:
            continue

        if index < 0:
            values = -values

        result_data[variable_name] = values
        if i < len(descriptions):
            result_desc[variable_name] = descriptions[i]

    return SimulationResult(
        data=result_data, descriptions=result_desc, time_variable=detected_time_name
    )


def load_csv(file_path: str | Path) -> SimulationResult:
    # Load CSV file
    file_path = Path(file_path)

    with file_path.open("r", newline="") as handle:
        reader = csv.reader(handle)
        headers = next(reader)

    # Validate headers
    headers = [header.strip() for header in headers]
    if not headers:
        raise ValueError("CSV file has no headers.")

    # Process data
    values = np.loadtxt(file_path, delimiter=",", skiprows=1)
    values = np.atleast_2d(values)

    if values.shape[1] != len(headers):
        raise ValueError(
            f"CSV column count mismatch. Headers={len(headers)}, data columns={values.shape[1]}"
        )

    data = {name: values[:, idx].reshape(-1) for idx, name in enumerate(headers)}

    time_variable = "time" if "time" in data else headers[0]
    return SimulationResult(data=data, descriptions={}, time_variable=time_variable)
