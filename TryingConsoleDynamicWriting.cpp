// TryingConsoleDynamicWriting.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <vector>
#include <cwchar>

std::wstring tetrimono[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;

int nScreenWidth = 80;
int nScreenHeight = 30;

int Rotate(int xc, int yc, int r) {
    switch (r % 4) {
    case 0: return yc * 4 + xc;
    case 1: return 12 + yc - (xc * 4);
    case 2: return 15 - (yc * 4) - xc;
    case 3: return 3 - yc + (xc * 4);
    }
    return 0;
}

bool DoesPieceFit(int nTetrimono, int nRotation, int nPosX, int nPosY) {
    
    for(int px = 0; px < 4; px++)
        for (int py = 0; py < 4; py++)
        {
            // Get index into piece
            int pi = Rotate(px, py, nRotation);

            // Get index into field
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight) 
                {
                    if (tetrimono[nTetrimono][pi] == L'*' && pField[fi] != 0)
                        return false; // fail on first hit
                }
            }
        }
    
    return true;
}

int main()
{
    /*std::cout << "Hello World!\n";
    std::wcout << "Long cout: " << tetrimono << std::endl;*/

    tetrimono[0].append(L"..*.");
    tetrimono[0].append(L"..*.");
    tetrimono[0].append(L"..*.");
    tetrimono[0].append(L"..*.");

    tetrimono[1].append(L"..*.");
    tetrimono[1].append(L".**.");
    tetrimono[1].append(L".*..");
    tetrimono[1].append(L"....");

    tetrimono[2].append(L".*..");
    tetrimono[2].append(L".**.");
    tetrimono[2].append(L"..*.");
    tetrimono[2].append(L"....");

    tetrimono[3].append(L"....");
    tetrimono[3].append(L".**.");
    tetrimono[3].append(L".**.");
    tetrimono[3].append(L"....");

    tetrimono[4].append(L"..*.");
    tetrimono[4].append(L".**.");
    tetrimono[4].append(L"..*.");
    tetrimono[4].append(L"....");

    tetrimono[5].append(L"....");
    tetrimono[5].append(L".**.");
    tetrimono[5].append(L"..*.");
    tetrimono[5].append(L"..*.");

    tetrimono[6].append(L"....");
    tetrimono[6].append(L".**.");
    tetrimono[6].append(L".*..");
    tetrimono[6].append(L".*..");

    pField = new unsigned char[nFieldWidth * nFieldHeight]; // Create play field bufffer
    for (int x = 0; x < nFieldWidth; x++)
        for (int y = 0; y < nFieldHeight; y++)
            pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Game Logic Stuff
    bool bGameOver = false;

    int nCourrentPiece = 0;
    int nCourrentRotation = 0;
    int nCourrentX = nFieldWidth / 2;
    int nCourrentY = 0;

    bool bKey[4];
    bool bRotateHold = false;

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForcePieceDown = false;
    int nPieceCount = 0;
    int nScore = 0;

    std::vector<int> vLines;

    while (!bGameOver) {

        // Game Timing ==============================
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        nSpeedCounter++;
        bForcePieceDown = (nSpeedCounter == nSpeed);
        
        // Input ====================================
        for (int k = 0; k < 4; k++)
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

        // Game Logic ===============================
        nCourrentX += (bKey[0] && DoesPieceFit(nCourrentPiece, nCourrentRotation, nCourrentX + 1, nCourrentY)) ? 1 : 0;
        nCourrentX -= (bKey[1] && DoesPieceFit(nCourrentPiece, nCourrentRotation, nCourrentX - 1, nCourrentY)) ? 1 : 0;
        nCourrentY += (bKey[2] && DoesPieceFit(nCourrentPiece, nCourrentRotation, nCourrentX, nCourrentY + 1)) ? 1 : 0;

        if (bKey[3]) {
            nCourrentRotation += (!bRotateHold && DoesPieceFit(nCourrentPiece, nCourrentRotation + 1, nCourrentX, nCourrentY)) ? 1 : 0;
            bRotateHold = true;
        }
        else
            bRotateHold = false;

        if (bForcePieceDown) {
            if (DoesPieceFit(nCourrentPiece, nCourrentRotation, nCourrentX, nCourrentY + 1))
                nCourrentY++;
            else {

                // Lock the current piece in the field
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++)
                        if(tetrimono[nCourrentPiece][Rotate(px,py,nCourrentRotation)] == L'*')
                            pField[(nCourrentY + py) * nFieldWidth + (nCourrentX + px)] = nCourrentPiece + 1;

                nPieceCount++;
                if (nPieceCount % 10 == 0)
                    if (nSpeed >= 10) nSpeed--;

                // Check have we got any lines

                for(int py = 0; py < 4; py++)
                    if (nCourrentY + py < nFieldHeight - 1) {
                        bool bLine = true;
                        for (int px = 0; px < nFieldWidth - 1; px++)
                            bLine &= (pField[(nCourrentY + py) * nFieldWidth + px]) != 0;
                        
                        if (bLine) {
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                pField[(nCourrentY + py) * nFieldWidth + px] = 8;

                            vLines.push_back(nCourrentY + py);
                        }
                    }

                nScore += 25;
                if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

                // Choose next piece
                nCourrentX = nFieldWidth / 2;
                nCourrentY = 0;
                nCourrentRotation = 0;
                nCourrentPiece = rand() % 7;

                // if piece does not fit
                bGameOver = !DoesPieceFit(nCourrentPiece, nCourrentRotation, nCourrentX, nCourrentY + 1);

            }
            
            nSpeedCounter = 0;
        }

        // Render Output ============================

        // Draw Field
        for (int x = 0; x < nFieldWidth; x++)
            for (int y = 0; y < nFieldHeight; y++)
                screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

        // Draw Current Piece
        for (int px = 0; px < 4; px++)
            for (int py = 0; py < 4; py++)
                if (tetrimono[nCourrentPiece][Rotate(px, py, nCourrentRotation)] == L'*')
                    screen[(nCourrentY + py + 2) * nScreenWidth + (nCourrentX + px + 2)] = nCourrentPiece + 65;

        if (!vLines.empty()) {
            // Display Frame (cheekily to draw lines)
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            for (auto &v : vLines)
                for (int px = 1; px < nFieldWidth - 1; px++)
                {
                    for (int py = v; py > 0; py--)
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    pField[px] = 0;
                }
            vLines.clear();
        }

        // Draw Score
        swprintf_s(&screen[2 + nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

        // Display Frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
    }

    // End of the Game
    CloseHandle(hConsole);
    std::cout << "Game over! Score: " << nScore << std::endl;
    system("pause");

    //std::wcout << "First tetrimono: " << tetrimono[0] << std::endl;

}