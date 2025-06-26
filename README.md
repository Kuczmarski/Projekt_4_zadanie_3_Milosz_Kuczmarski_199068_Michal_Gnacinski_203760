# Zadanie 3 - Symulator windy

## Spis treści

1. [Wstęp](#wstęp)
2. [Opis działania](#opis-działania)
3. [Funkcje](#funkcje)
4. [Instrukcja kompilacji](#instrukcja-kompilacji)
5. [Testy i wyniki](#testy-i-wyniki)
6. [Autorzy](#autorzy)

---

## Wstęp

Projekt przedstawia symulator windy w budynku pięciokondygnacyjnym. Aplikacja wizualizuje ruch kabiny oraz sposób obsługi zgłoszeń pasażerów.

## Opis działania

Użytkownik klika przyciski na wybranym piętrze, aby wezwać windę i określić cel podróży. System zbiera zgłoszenia w kolejce, przemieszcza kabinę między piętrami, a także monitoruje obciążenie (limit 600 kg).

## Funkcje

- **ObliczY(int pietro)**

  - Oblicza wartość współrzędnej Y w oknie dla danego piętra.

- **ObsluzZgloszenie(int start, int cel)**

  - Dodaje żądanie wejścia i wyjścia pasażera do kolejki, sprawdza limit obciążenia.

- **ObsluzPrzyjazd(int pietro)**

  - Obsługa przyjazdu kabiny: wysiadanie, wsiadanie nowych pasażerów, aktualizacja kolejki.

- **Rysuj(HDC hdc)**

  - Rysuje szyby, kabinę, pasażerów oraz aktualny stan obciążenia.

- **WndProc**

  - Główna procedura komunikatów WinAPI (tworzenie przycisków, obsługa WM\_COMMAND, WM\_TIMER, WM\_PAINT).

## Instrukcja kompilacji

1. Otwórz wiersz poleceń i przejdź do katalogu zawierającego `main.cpp`.
2. Uruchom kompilator MSVC:
   ```batch
   cl main.cpp /EHsc user32.lib gdi32.lib ole32.lib gdiplus.lib
   main.exe
   ```

## Testy i wyniki

Przeprowadzono następujące scenariusze testowe:

- **Pojedynczy pasażer** – wezwanie windy z parteru na piętro 3: kabina prawidłowo podjeżdża i wysadza pasażera.
- **Przeciążenie** – próba wejścia więcej niż \~8 pasażerów (9×70 kg = 630 kg): ostatni pasażer nie wsiada.
- **Kolejkowanie** – zgłoszenia z różnych pięter: winda odwiedza piętra w kolejności zgłoszeń.
- **Powrót na parter** – po obsłużeniu wszystkich przystanków kabina wraca automatycznie na parter po kilku sekundach bezczynności.

## Autorzy

- Miłosz Kuczmarski (nr indeksu: 199068)
- Michał Gnaciński (nr indeksu: 203760)

