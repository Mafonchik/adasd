#!/usr/bin/env python3

import os
import argparse

def count_exe_files(directory):
    count = 0
    for root, _, files in os.walk(directory):
        for file in files:
            if file.lower().endswith(".exe"):
                count += 1
    return count

def main():
    parser = argparse.ArgumentParser(
        description="Рекурсивно подсчитывает количество исполняемых (.exe) файлов в указанной директории и её поддиректориях.",
        usage="%(prog)s [название директории]"
    )
    parser.add_argument(
        "directory",
        nargs="?",  # Делаем аргумент необязательным
        default=".",  # Значение по умолчанию — текущая директория
        help="Директория для поиска .exe файлов (по умолчанию: текущая директория)."
    )
    args = parser.parse_args()

    if not os.path.isdir(args.directory):
        print(f"Ошибка: директория '{args.directory}' не найдена. Проверьте введенные данные.")
        sys.exit(1)

    count = count_exe_files(args.directory)

    print(f"Найдено .exe файлов: {count}.")

if __name__ == "__main__":
    main()
