#!/bin/sh

usage() {
    echo "Рекурсивно подсчитывает количество исполняемых (.exe) файлов в указанной директории и её поддиректориях."
    echo ""
    echo "Использование: $0 [directory]"
    echo ""
    echo "positional arguments:"
    echo "  directory  Директория для поиска .exe файлов (по умолчанию: текущая директория)."
    echo ""
    echo "options:"
    echo "  -h, --help    Показать это сообщение."
    exit 0
}


for arg in "$@"; do
    if [[ "$arg" == "-h" || "$arg" == "--help" ]]; then
        usage
    fi
done

if [ -z "$1" ]; then
    DIR="."
else
    DIR="$1"
fi

#Проверка на существование директории
if [ ! -d "$DIR" ]; then
    echo "Ошибка: директории $DIR не найдено. Проверьте введенные данные"
    exit 1
fi

ans=$(find "$DIR" -type f -iname "*.exe" | wc -l)
echo "Найдено .exe файлов: $ans"
