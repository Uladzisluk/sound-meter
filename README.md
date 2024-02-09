# sound-meter

Żeby uruchomic w Visual Studio należy:

1. Stworzyć projekt
2. Skopiować main.cpp do projektu
3. W "Properties" projektu przejść do sekcji "Configuration Properties" -> "C/C++" -> "General"
4. W polu "Additional Include Directories" dodać ścieżkę do plików nagłówkowych WinAPI, np. "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\ucrt"
5. W sekcji "Configuration Properties" -> "Linker" -> "Input" w polu "Additional Dependencies" dodać "winmm.lib"
6. Skompilować i uruchomić projekt
