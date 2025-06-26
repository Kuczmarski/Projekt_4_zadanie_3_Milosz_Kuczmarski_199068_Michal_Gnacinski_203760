# Symulator windy

## Spis treści
1. [Wstęp](#wstęp)
2. [Opis działania](#opis-działania)
3. [Implementacja](#implementacja)
4. [Instrukcja kompilacji](#instrukcja-kompilacji)
5. [Testy i wyniki](#testy-i-wyniki)
6. [Podsumowanie](#podsumowanie)

---

## Wstęp
Ten projekt to prosty symulator windy w budynku (parter i 4 piętra). Aplikacja pokazuje ruch kabiny i obsługę zgłoszeń pasażerów.

## Opis działania
Użytkownik może kliknąć przyciski na dowolnym piętrze, aby wezwać windę i wskazać, na które piętro chce się dostać. Winda jeździ według kolejki zgłoszeń, pokazując przy tym aktualne obciążenie kabiny.

## Implementacja

### Struktura pliku
Całość kodu znajduje się w jednym pliku `main.cpp`. Użyto WinAPI i GDI+ do stworzenia okna, przycisków oraz rysowania grafiki.

### Kluczowe funkcje
- **ObliczY(pietro)**: oblicza pozycję pionową kabiny dla danego piętra.
- **ObsluzZgloszenie(start, cel)**: dodaje żądanie wywołania windy do odpowiednich kolejek.
- **ObsluzPrzyjazd(pietro)**: obsługuje wysiadanie i wsiadanie pasażerów przy przyjeździe.
- **Rysuj(hdc)**: rysuje szybę, kabinę i pasażerów oraz licznik obciążenia.
- **WndProc**: główna procedura obsługi zdarzeń (przyciski, timery, malowanie).

## Instrukcja kompilacji
```bash
cd ścieżka/do/pliku
cl main.cpp /EHsc user32.lib gdi32.lib ole32.lib gdiplus.lib
# Uruchomienie:
main.exe
