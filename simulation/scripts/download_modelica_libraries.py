import urllib.request
import tempfile
from pathlib import Path
import zipfile
import shutil


def main():
    # Variables
    download_url = "https://github.com/AlexHendersonEng/main/archive/6da0f665a0be5986111b4971077bdf0a9f83fbd6.zip"
    modelica_dir = Path(__file__).parents[1] / "modelica"
    modelica_dir.mkdir(parents=True, exist_ok=True)
    (modelica_dir / ".gitignore").write_text("*")

    # Create a temporary directory for download
    with tempfile.TemporaryDirectory() as temp_dir:
        temp_dir = Path(temp_dir)
        zip_file = temp_dir / "main.zip"
        extract_dir = temp_dir / "main"

        # Download the zip file
        urllib.request.urlretrieve(download_url, zip_file)

        # Unzip the downloaded data
        with zipfile.ZipFile(zip_file) as zip_file:
            zip_file.extractall(extract_dir)

        # Copy the extracted files to the modelica directory
        shutil.copytree(
            extract_dir
            / "main-6da0f665a0be5986111b4971077bdf0a9f83fbd6"
            / "modelica_aerospace"
            / "ModelicaAerospace",
            modelica_dir / "ModelicaAerospace",
            dirs_exist_ok=True,
        )
        shutil.copytree(
            extract_dir
            / "main-6da0f665a0be5986111b4971077bdf0a9f83fbd6"
            / "modelica_automotive"
            / "ModelicaAutomotive",
            modelica_dir / "ModelicaAutomotive",
            dirs_exist_ok=True,
        )
        shutil.copytree(
            extract_dir
            / "main-6da0f665a0be5986111b4971077bdf0a9f83fbd6"
            / "modelica_maritime"
            / "ModelicaMaritime",
            modelica_dir / "ModelicaMaritime",
            dirs_exist_ok=True,
        )


if __name__ == "__main__":
    main()
