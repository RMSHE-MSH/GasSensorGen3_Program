#include "Universal.h"
using namespace std;

double width = 16, high = 16;
//#define step 0.22
//#define step 0.107
#define step 0.095
//#define step 0.06
//#define step 0.12
//#define step 0.22

#define pi 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679

double f(double x) {
    return 8 * (sin(x - (0.8 * cos(x - 1.5))) + 1);
}

double f1(double x) {
    return 30 * (sin(x - (0.8 * cos(x - 1.5))) + 1);
}

string bin2hex(string bin) {
    string hex = "";
    for (int i = 0; i < bin.length(); i += 4) {
        int val = 0;
        for (int j = i; j < i + 4; j++) {
            val = val * 2 + (bin[j] - '0');
        }
        hex += (val < 10) ? ('0' + val) : ('A' + val - 10);
    }
    return hex;
}

string toLower(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

void out(int i) {
    std::cout << "{";
    if (width <= 8 || high <= 8) {
        for (int x = 0; x < width; ++x) {
            string List = "";
            for (int y = 0; y < high; ++y) {
                COLORREF C = getpixel(x, y);
                if (GetRValue(C) >= 128) {
                    List = "1" + List;
                } else {
                    List = "0" + List;
                }
            }
            cout << "0x" << toLower(bin2hex(List)) << ",";
        }
    }

    if (width > 8 || high > 8) {
        for (int x = 0; x < width; ++x) {
            string List = "";
            for (int y = 0; y < high / 2; ++y) {
                COLORREF C = getpixel(x, y);
                if (GetRValue(C) >= 128) {
                    List = "1" + List;
                } else {
                    List = "0" + List;
                }
            }
            cout << "0x" << toLower(bin2hex(List)) << ",";
        }

        for (int x = 0; x < width; ++x) {
            string List = "";
            for (int y = high / 2; y < high; ++y) {
                COLORREF C = getpixel(x, y);
                if (GetRValue(C) >= 128) {
                    List = "1" + List;
                } else {
                    List = "0" + List;
                }
            }
            cout << "0x" << toLower(bin2hex(List)) << ",";
        }
    }

    std::cout << "},\t// Loading-[" << i << "];" << endl;
}

int main() {
    initgraph(width, high, EW_SHOWCONSOLE);
    //setlinestyle(PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_ROUND, 2);

    Sleep(1000);

    for (int i = 0; i < 1; ++i) {
        double x = -2.2343286332277, x1 = 0.5, x2 = 0, x3 = pi / 1.25;
        for (int i = 0; i < 35000000; ++i) {
            if (i >= 5) {
                cleardevice();
                BeginBatchDraw();
                //solidrectangle(f(x) - (PX / pi * sin(x - pi / 2)), f(x) - (PX / pi * sin(x - pi / 2)), f(x) + (PX / pi * sin(x - pi / 2)), f(x) + (PX / pi * sin(x - pi / 2)));

                solidroundrect(f(x) - (width / pi * sin(x1 - pi / 2)), f(x) - (high / pi * sin(x1 - pi / 2)), f(x) + (width / pi * sin(x1 - pi / 2)), f(x) + (high / pi * sin(x1 - pi / 2)), 4 * (sin(x3 - pi / 2) + 1) * (cos(x2) + 1), 4 * (sin(x3 - pi / 2) + 1) * (cos(x2) + 1));

                //solidroundrect(f1(x) + (width / pi * sin(x1 - pi / 2)), 0, f1(x) + (width / pi * sin(x1 - pi / 2)) + 6 * f(x), high, 0, 0);

                //solidroundrect(f1(x) + (width / pi * sin(x1 - pi / 2)), 0, f1(x) + (width / pi * sin(x1 - pi / 2)) + 6 * f(x), high, 0, 0);

                setlinecolor(RGB(0, 0, 0));
                roundrect(1, 1, width - 2, high - 2, 0, 0);

                setlinecolor(RGB(255, 255, 255));
                roundrect(0, 0, width - 1, high - 1, 0, 0);

                //Sleep(1);
                EndBatchDraw();

                //saveimage(_T("E:\\test.bmp"));
                //saveimage(_T((to_string(i) + ".bmp").c_str()));

                out(i);
            }
            x += step;
            x1 += step;
            x2 += step;
            x3 += step;
        }
    }

    while (1) {}
}