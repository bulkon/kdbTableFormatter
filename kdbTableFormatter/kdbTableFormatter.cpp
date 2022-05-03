#include <string>
#include <iostream>
#include <ctime>

#define KXVER 3

#include "k.h"

using namespace std;

I up(J f) { return (f / 8.64e13 + 10957) * 8.64e4; }  // unix from kdb+ timestamp
// Convert time to hour, minute, seconds, miliseconds
void tsms(unsigned ts, char* h, char* m, char* s, short* mmm) { *h = ts / 3600000; ts -= 3600000 * (*h); *m = ts / 60000; ts -= 60000 * (*m); *s = ts / 1000; ts -= 1000 * (*s); *mmm = ts; }
// Format time to %H:%M:%S.%s
char* ftsms(unsigned ts, char* d, int size) { char h, m, s; short mmm; tsms(ts, &h, &m, &s, &mmm); sprintf_s(d, size, "%02d:%02d:%02d.%03d", h, m, s, mmm); return d; }

string serialize_table(K data, string type) {
    int row, col;

    K columnNames = kK(data->k)[0];
    K columnData = kK(data->k)[1];

    int nCols = columnNames->n;
    int nRows = kK(columnData)[0]->n;

    string res_s = type;

    if (nRows == 0)
        return "null";

    for (row = 0; row < nRows; row++) {

        if (0 == row) {
            string cs = "";
            for (col = 0; col < nCols; col++) {
                if (col == 0)
                    res_s = res_s + kS(columnNames)[col];
                else
                    res_s = res_s + "," + kS(columnNames)[col];
            }
            res_s = res_s + "\n";
            printf("\n");
        }
        for (col = 0; col < nCols; col++) {
            K obj = kK(columnData)[col];
            printf("Type of Kx fields received: %d\n", obj->t);

            if (col > 0)
                res_s = res_s + ",";
            switch (obj->t) {
            case(0): {// Mixed List
                for (int idx = 0; idx < obj->n; idx++) {
                    K elem = kK(obj)[idx];

                    switch (elem->t)
                    {
                    case(99): {
                        K keys = kK(elem)[0];
                        K vals = kK(elem)[1];
                        for (int i = 0; i < keys->n; i++) {
                            res_s = res_s + "`" + kS(keys)[i];
                        }
                        res_s = res_s + "!(";
                        for (int i = 0; i < vals->n; ++i)
                        {
                            K x = kK(vals)[i];
                            switch (x->t)
                            {
                            case(-1): {res_s += to_string(x->g); }break;
                            case(-2): {

                                for (int i = 0; i < 16; i++) {
                                    res_s += to_string(x->u);
                                }

                            }break;
                            case(-4): {res_s += to_string(x->g); }break;
                            case(-5): {res_s += to_string(x->h); }break;
                            case(-6): {res_s += to_string(x->i); }break;
                            case(-7): {res_s += to_string(x->j); }break;
                            case(-8): {res_s += to_string(x->e); }break;
                            case(-9): {res_s += to_string(x->f); }break;
                            case(-11): {res_s += x->s; }break;
                            case(-12): {
                                time_t time = (time_t)up(x->j);
                                tm ptm;
                                errno_t err;
                                err = gmtime_s(&ptm, &time);
                                if (err)
                                {
                                    printf("Invalid argument to localtime_s.");
                                    exit(1);
                                }
                                char buffer[32];
                                strftime(buffer, 32, "%Y.%m.%d %T", &ptm);

                                res_s += buffer;
                            }break;
                            case(-14): {res_s += to_string(x->i); }break;
                            case(-16): {res_s += to_string(x->j); }break;
                            case(-19): {
                                char buf[32];
                                res_s += ftsms(x->i, buf, 32);
                            }break;
                            case(10): {int i; for (i = 0; i < xn; i++) res_s += kC(x)[i]; }break;
                            }

                            if (i != vals->n - 1)
                                res_s += ";";

                        }
                        res_s += ")";
                    }break;
                    case(10): {
                        int i;
                        K x = elem;
                        for (i = 0; i < xn; i++) {
                            res_s += kC(x)[i];
                        }
                    }break;
                    default: {
                        printf("type %d not supported", obj->t);
                    }
                    }
                }
            }  break;
            case (1): {
                res_s = res_s + to_string(kG(obj)[row]);
            }
                    break;
            case (2): {
                U g = kU(obj)[row];
                char buff[32];
                for (int i = 0; i < 16; i++) {
                    sprintf_s(buff, 32, "%02x", g.g[i]);
                    res_s += buff;
                }
            }
                    break;
            case (4): {
                res_s = res_s + std::to_string(kG(obj)[row]);

            }
                    break;
            case (5): {
                res_s = res_s + to_string(kH(obj)[row]);
            }
                    break;
            case (6): {
                res_s = res_s + to_string(kI(obj)[row]);
            }
                    break;
            case (7): {
                res_s = res_s + to_string(kJ(obj)[row]);
            }
                    break;
            case (8): {
                res_s = res_s + to_string(kE(obj)[row]);
            }
                    break;
            case (9): {
                res_s = res_s + to_string(kF(obj)[row]);
            }
                    break;
            case (11): {
                res_s = res_s + kS(obj)[row];
            }
                     break;
            case (12): {
                time_t time = (time_t)up(kJ(obj)[row]);
                tm ptm;
                errno_t err;
                err = gmtime_s(&ptm, &time);
                if (err)
                {
                    printf("Invalid argument to localtime_s.");
                    exit(1);
                }
                char buffer[32];
                strftime(buffer, 32, "%Y.%m.%d %T", &ptm);

                res_s += buffer;
            }
                     break;
            case (19): {
                char buf[32];
                res_s += ftsms(kI(obj)[row], buf, 32);
            }
                     break;

            default: {
                printf("type %d not supported", obj->t);
            }
                   break;
            }
        }
        if (row < nRows - 1)
            res_s = res_s + ":";
    }
    return res_s;
}

int main() {
    char hostname[30] = "localhost";
    char creds[30] = "user:password";
    int handle = khpu(hostname, 14018, creds);

    char q[200] = "1 sublist trades_secure";
    K r = k(handle, q, (K)0);
    string tblString = serialize_table(r, "");
    cout << tblString;
}
