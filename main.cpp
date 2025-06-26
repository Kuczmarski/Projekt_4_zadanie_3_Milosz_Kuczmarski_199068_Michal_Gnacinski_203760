//Symulator windy - main.cpp
//Kompilacja:
// cd miejsce pliku
// cl main.cpp /EHsc user32.lib gdi32.lib ole32.lib gdiplus.lib

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;

//Stałe konfiguracyjne
enum {
    LICZBA_PIETER = 5,
    SZER_OKNA = 1600,
    WYS_OKNA = 1200,
    POZ_SZYBU_X = 600,
    SZER_SZYBU = 240,
    MARGINES = 20,
    PRZYCISK_SZER = 60,
    PRZYCISK_WYS = 30,
    PRZERWA_PRZYC = 8,
    ID_TIMER_RUCH = 1,
    ID_TIMER_PUSTA = 2,
    INTERWAL_RUCH = 40,
    OPOZ_PUSTA = 5000,
    PREDKOSC_PX = 6
};
constexpr double SR_WAGA = 70.0;   // srednia waga pasażera
constexpr double MAX_CIEZAR = 600.0;  // maksymalne obciążenie windy

enum { ID_BTN_START = 1000, ID_STATUS = 2000 };

//Zmienne globalne
HINSTANCE   hInst;
HWND        hOkno;
HWND        hStatus;
ULONG_PTR   tokenGdi;
int         wysokoscPietra;
int         obecnePietro = 0;
int         polozenieY;
int         liczbaPasazerow = 0;

//kolejki
vector<vector<int>> listaOczekujacych(LICZBA_PIETER);
vector<int>          listaPrzystankow;
vector<int>          listaCelowWWindzie;

//Oblicza współrzędną Y dla danego pietra
inline int ObliczY(int pietro);
//Oblicza aktualne obciazenie windy
inline double ObliczObciazenie();
//Aktualizuje licznik wagi
void AktualizujWage();
//Obsluguje nowe zgłoszenie: start i cel pietra
void ObsluzZgloszenie(int start, int cel);
//Obsługuje przyjazd windy na dane pietro
void ObsluzPrzyjazd(int pietro);
//Rysuje wszystko
void Rysuj(HDC hdc);
inline int ObliczY(int pietro) {
    return MARGINES + (LICZBA_PIETER - 1 - pietro) * wysokoscPietra;
}

inline double ObliczObciazenie() {
    return liczbaPasazerow * SR_WAGA;
}

void AktualizujWage() {
    wostringstream ss;
    ss << L"Aktualne obciążenie: " << fixed << setprecision(1)
        << ObliczObciazenie() << L" kg";
    SetWindowTextW(hStatus, ss.str().c_str());
}

//Logika obsługi zgłoszeń
void ObsluzZgloszenie(int start, int cel) {
    // Walidacja zakresu pięter
    if (start < 0 || start >= LICZBA_PIETER || cel < 0 || cel >= LICZBA_PIETER)
        return;
    listaOczekujacych[start].push_back(cel);
    //Sprawdza czy winda jest wolna
    if (listaPrzystankow.empty() && start == obecnePietro) {
        int docel = listaOczekujacych[start].front();
        listaOczekujacych[start].erase(listaOczekujacych[start].begin());
        //Sprawdzane limitu wagi
        if ((liczbaPasazerow + 1) * SR_WAGA <= MAX_CIEZAR) {
            liczbaPasazerow++;
            listaCelowWWindzie.push_back(docel);
            listaPrzystankow.push_back(docel);
            AktualizujWage();
            SetTimer(hOkno, ID_TIMER_RUCH, INTERWAL_RUCH, nullptr);
        }
        else {
            listaOczekujacych[start].insert(listaOczekujacych[start].begin(), docel);
        }
    }
    else {
        bool jedzie = !listaPrzystankow.empty();
        listaPrzystankow.push_back(start);
        listaPrzystankow.push_back(cel);
        if (!jedzie) {
            //start timera ruchu
            SetTimer(hOkno, ID_TIMER_RUCH, INTERWAL_RUCH, nullptr);
        }
    }
}

//Obsługa przyjazdu do piętra
void ObsluzPrzyjazd(int pietro) {
    KillTimer(hOkno, ID_TIMER_RUCH);  //Zatrzymaj timer ruchu
    obecnePietro = pietro;
    polozenieY = ObliczY(pietro);

    //Wysiadanie pasazerow
    vector<int> pozostali;
    for (int c : listaCelowWWindzie) {
        if (c == pietro)
            liczbaPasazerow--;  //odjecie osob i wagi
        else
            pozostali.push_back(c);
    }
    listaCelowWWindzie.swap(pozostali);  
    listaPrzystankow.erase(
        remove(listaPrzystankow.begin(), listaPrzystankow.end(), pietro),
        listaPrzystankow.end());

    //wsiadanie pasazerow ktorzy czekaja, jesli waga jest ok
    vector<int> temp;
    for (int c : listaOczekujacych[pietro]) {
        if ((liczbaPasazerow + 1) * SR_WAGA <= MAX_CIEZAR) {
            liczbaPasazerow++;
            listaCelowWWindzie.push_back(c);
            listaPrzystankow.push_back(c);
        }
        else {
            temp.push_back(c); 
        }
    }
    listaOczekujacych[pietro].swap(temp);

    AktualizujWage();
    InvalidateRect(hOkno, nullptr, FALSE);  //odswiezenia okna
    if (!listaPrzystankow.empty()) {
        SetTimer(hOkno, ID_TIMER_RUCH, INTERWAL_RUCH, nullptr);
    }
    else {
        //jesli nie ma przystanków, sprawdź inne piętra z oczekującymi
        vector<int> pietra;
        for (int f = 0; f < LICZBA_PIETER; ++f)
            if (!listaOczekujacych[f].empty())
                pietra.push_back(f);
        if (!pietra.empty()) {
            sort(pietra.begin(), pietra.end());
            for (int f : pietra)
                listaPrzystankow.push_back(f);
            SetTimer(hOkno, ID_TIMER_RUCH, INTERWAL_RUCH, nullptr);
        }
        else {
            //powrot na parter
            SetTimer(hOkno, ID_TIMER_PUSTA, OPOZ_PUSTA, nullptr);
        }
    }
}

//rysowanie interfejsu
void Rysuj(HDC hdc) {
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeHighQuality);
    g.Clear(Color::White);
    FontFamily ff(L"Segoe UI");
    Font fontP(&ff, 28, FontStyleBold);
    SolidBrush brP(Color(50, 50, 50));
    StringFormat sf;
    sf.SetAlignment(StringAlignmentNear);
    sf.SetLineAlignment(StringAlignmentCenter);
    for (int f = 0; f < LICZBA_PIETER; ++f) {
        int y = ObliczY(f) + wysokoscPietra / 2;
        wchar_t buf[3]; swprintf(buf, 3, L"%d", f);
        g.DrawString(buf, -1, &fontP,
            RectF(POZ_SZYBU_X + SZER_SZYBU + 10, y - 14, 30, 30),
            &sf, &brP);
    }
    Pen penL(Color(200, 200, 200), 1);
    for (int i = 0; i <= LICZBA_PIETER; ++i)
        g.DrawLine(&penL, POZ_SZYBU_X, ObliczY(i), POZ_SZYBU_X + SZER_SZYBU, ObliczY(i));
    Pen penB(Color::Black, 3);
    g.DrawRectangle(&penB, POZ_SZYBU_X, MARGINES,
        SZER_SZYBU, wysokoscPietra * LICZBA_PIETER);
    // Rysuje pietra i oczekujących pasażerów jako niebieskie kółka
    Pen penPlat(Color(150, 150, 150), 2);
    int px = POZ_SZYBU_X + SZER_SZYBU + 40;
    Font fontK(&ff, 16, FontStyleBold);
    SolidBrush brK(Color(0, 120, 215));
    SolidBrush brKT(Color::White);
    StringFormat sfc;
    sfc.SetAlignment(StringAlignmentCenter);
    sfc.SetLineAlignment(StringAlignmentCenter);
    for (int fl = 0; fl < LICZBA_PIETER; ++fl) {
        int y = ObliczY(fl) + wysokoscPietra - 8;
        g.DrawLine(&penPlat, px, y, SZER_OKNA - MARGINES, y);
        for (size_t i = 0; i < listaOczekujacych[fl].size(); ++i) {
            RectF r(px + 10 + i * (PRZYCISK_SZER + PRZERWA_PRZYC) - PRZYCISK_WYS,
                y - PRZYCISK_WYS, PRZYCISK_WYS * 2, PRZYCISK_WYS * 2);
            g.FillEllipse(&brK, r);
            wchar_t t[3]; swprintf(t, 3, L"%d", listaOczekujacych[fl][i]);
            g.DrawString(t, -1, &fontK, r, &sfc, &brKT);
        }
    }
    //rysuje kabine windy i pasazerow jako zielone kolka
    SolidBrush brC(Color(200, 200, 200));
    Rect kab(POZ_SZYBU_X + 4, polozenieY + MARGINES + 4,
        SZER_SZYBU - 8, wysokoscPietra - 8);
    g.FillRectangle(&brC, kab);
    SolidBrush brPz(Color(0, 180, 50));
    for (size_t i = 0; i < listaCelowWWindzie.size(); ++i) {
        RectF r(kab.X + 20 + i * 28 - 10,
            kab.Y + kab.Height / 2 - 10, 20, 20);
        g.FillEllipse(&brPz, r);
        g.DrawString(to_wstring(listaCelowWWindzie[i]).c_str(),
            -1, &fontK, r, &sfc, &brKT);
    }
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        hOkno = hwnd;
        hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        RECT rc; GetClientRect(hwnd, &rc);
        wysokoscPietra = (rc.bottom - 2 * MARGINES) / LICZBA_PIETER;
        polozenieY = ObliczY(0);
        //Tworzy przyciski do windy dla kazdego pietra
        for (int fl = 0; fl < LICZBA_PIETER; ++fl) {
            int y = ObliczY(fl) + (wysokoscPietra - PRZYCISK_WYS) / 2;
            int idx = 0;
            for (int d = 0; d < LICZBA_PIETER; ++d) if (d != fl) {
                CreateWindowW(L"BUTTON", to_wstring(d).c_str(), WS_CHILD | WS_VISIBLE,
                    MARGINES + idx * (PRZYCISK_SZER + PRZERWA_PRZYC), y,
                    PRZYCISK_SZER, PRZYCISK_WYS,
                    hwnd, HMENU(ID_BTN_START + fl * LICZBA_PIETER + d), hInst, nullptr);
                ++idx;
            }
        }
        //status wagi w kabinie
        hStatus = CreateWindowW(L"STATIC", L"Waga w windzie: 0 kg",
            WS_CHILD | WS_VISIBLE, MARGINES, MARGINES, 400, 40,
            hwnd, (HMENU)ID_STATUS, hInst, nullptr);
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp);
        if (id >= ID_BTN_START && id < ID_BTN_START + LICZBA_PIETER * LICZBA_PIETER) {
            int rel = id - ID_BTN_START;
            ObsluzZgloszenie(rel / LICZBA_PIETER, rel % LICZBA_PIETER);
        }
        break;
    }
    case WM_TIMER: {
        if (wp == ID_TIMER_RUCH) {
            if (listaPrzystankow.empty()) {
                KillTimer(hwnd, ID_TIMER_RUCH);
                break;
            }
            int cel = listaPrzystankow.front();
            bool wGore = (cel > obecnePietro);
            int staryY = polozenieY;
            //przesuwanie windy
            if (wGore)
                polozenieY -= PREDKOSC_PX;
            else
                polozenieY += PREDKOSC_PX;
            InvalidateRect(hwnd, nullptr, FALSE);
            int yDoc = ObliczY(cel);
            for (int c : listaCelowWWindzie) {
                int yC = ObliczY(c);
                if ((wGore && staryY > yC && polozenieY <= yC) ||
                    (!wGore && staryY < yC && polozenieY >= yC)) {
                    ObsluzPrzyjazd(c);
                    goto KoniecTimer;
                }
            }
            if ((wGore && polozenieY <= yDoc) || (!wGore && polozenieY >= yDoc)) {
                ObsluzPrzyjazd(cel);
            }
        KoniecTimer:;
        }
        else if (wp == ID_TIMER_PUSTA) {
            //jesli winda nie ma pasazerow, po przerwie wraca na parter
            KillTimer(hwnd, ID_TIMER_PUSTA);
            if (obecnePietro != 0 && listaPrzystankow.empty()) {
                listaPrzystankow.push_back(0);
                SetTimer(hwnd, ID_TIMER_RUCH, INTERWAL_RUCH, nullptr);
            }
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, SZER_OKNA, WYS_OKNA);
        HBITMAP oldBmp = (HBITMAP)SelectObject(mem, bmp);
        Rysuj(mem);  
        BitBlt(hdc, 0, 0, SZER_OKNA, WYS_OKNA, mem, 0, 0, SRCCOPY);
        SelectObject(mem, oldBmp);
        DeleteObject(bmp);
        DeleteDC(mem);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nShow) {
    hInst = hInstance;
    GdiplusStartupInput gi;
    GdiplusStartup(&tokenGdi, &gi, nullptr);  //inicjalizacja GDI+
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"ElevatorSim";
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowExW(
        0, L"ElevatorSim", L"Symulator windy",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        SZER_OKNA, WYS_OKNA,
        nullptr, nullptr, hInstance, nullptr);
    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    GdiplusShutdown(tokenGdi);
    return (int)msg.wParam;
}
