#include <string>
#include <iostream>
#include <string.h> 

#include <chrono>
#include <time.h>

using namespace std;

// ----------------------------------------------------------------------
void splitNibbles(char byte, char& nibbleH, char& nibbleL) {
  nibbleH = (byte & 0xF0) >> 4;
  nibbleL = (byte & 0xF);
}

// ----------------------------------------------------------------------
char ascii(char x) {
  if (0x0 == x) return 0x30;
  if (0x1 == x) return 0x31;
  if (0x2 == x) return 0x32;
  if (0x3 == x) return 0x33;
  if (0x4 == x) return 0x34;
  if (0x5 == x) return 0x35;
  if (0x6 == x) return 0x36;
  if (0x7 == x) return 0x37;
  if (0x8 == x) return 0x38;
  if (0x9 == x) return 0x39;
  if (0xa == x) return 0x41;
  if (0xb == x) return 0x42;
  if (0xc == x) return 0x43;
  if (0xd == x) return 0x44;
  if (0xe == x) return 0x45;
  if (0xf == x) return 0x46;
  return 'X';
}

// ----------------------------------------------------------------------
char* printString(char* data, int size) {
  char *sprt = new char[2*size+1];
  char nibbleH, nibbleL;
  for (int i = 0; i < size; ++i) {
    splitNibbles(data[i], nibbleH, nibbleL);
    sprt[2*i]     = ascii(nibbleH);
    sprt[2*i + 1] = ascii(nibbleL);
  }
  sprt[2*size] = '\0';
  return sprt;
}

// ----------------------------------------------------------------------
void putInt(int intVal, char *p) {
  memcpy(p, &intVal, sizeof(int));
}

// ----------------------------------------------------------------------
int getInt(char *p, int len) {
  int intVal(0);
  memcpy(&intVal, p, sizeof intVal);
  return intVal;
}

// ----------------------------------------------------------------------
int getFloat(char *p, int len) {
  float floatVal(0);
  memcpy(&floatVal, p, sizeof floatVal);
  return floatVal;
}

// ----------------------------------------------------------------------
double rd(){
  return static_cast<double> (rand()/(static_cast<double> (RAND_MAX/800)))-400;
}

// ----------------------------------------------------------------------
struct blobData {
  int sid;
  double vx, vy, vz;
  double colx, coly, colz;
  double rowx, rowy, rowz;
  char* serialize() {
    int offset(0);
    char *sd = new char[sizeof(blobData)+1];
    cout << " sizeof(blobData) = " << sizeof(blobData)
         << " sizeof(int) = " << sizeof(int)
         << " sizeof(double) = " << sizeof(double)
         << endl;
    // -- sid
    cout << "serialize sid = " << sid  << endl;
    memcpy(sd, &sid, sizeof(int));
    offset += sizeof(int);
    // -- vx,vy,vz
    cout << "serialize vx = " << vx
         << " vy = " << vy 
         << " vz = " << vz
         << endl;
    memcpy(sd + offset, &vx, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &vy, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &vz, sizeof(double));  offset += sizeof(double);

    // -- colx,coly,colz
    cout << "serialize colx = " << colx
         << " coly = " << coly 
         << " colz = " << colz
         << endl;
    memcpy(sd + offset, &colx, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &coly, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &colz, sizeof(double));  offset += sizeof(double);

    // -- rowx,rowy,rowz
    cout << "serialize rowx = " << rowx
         << " rowy = " << rowy 
         << " rowz = " << rowz
         << endl;
    memcpy(sd + offset, &rowx, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &rowy, sizeof(double));  offset += sizeof(double);
    memcpy(sd + offset, &rowz, sizeof(double));  offset += sizeof(double);

    sd[offset] = '\0';
    cout << "serialize sd ->" << printString(sd, offset) << endl;
    return sd;
  }
  blobData deSerialize(char *pdata) {
    blobData a;
    int offset(0);
    memcpy(&sid, pdata, sizeof(int));
    cout << "deserialize sid = " << sid << endl;
    offset = sizeof(int);
    memcpy(&vx, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&vy, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&vz, pdata + offset, sizeof(double)); offset += sizeof(double); 

    memcpy(&colx, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&coly, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&colz, pdata + offset, sizeof(double)); offset += sizeof(double); 

    memcpy(&rowx, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&rowy, pdata + offset, sizeof(double)); offset += sizeof(double); 
    memcpy(&rowz, pdata + offset, sizeof(double)); offset += sizeof(double); 

    return a;
  }

  void print() {
    cout << "sid = " << sid
         << " v(x/y/z) = (" << vx << "/" << vy << "/" << vz << ")"
         << " c(x/y/z) = (" << colx << "/" << coly << "/" << colz << ")"
         << " r(x/y/z) = (" << rowx << "/" << rowy << "/" << rowz << ")"
         << endl;
  }
};

// ----------------------------------------------------------------------
int main(int argc, const char **argv) {

  // -- command line arguments
  for (int i = 0; i < argc; i++){
    // if (!strcmp(argv[i],"-c"))  {doMake = true;}
    // if (!strcmp(argv[i],"-f"))  {first   = atoi(argv[++i]);}
  }

  int intVal(1);
  char data[4];
  if (0) {
    memcpy(data+0, "DA", 1);
    memcpy(data+1, "ED", 1);
    memcpy(data+2, "FA", 1);
    memcpy(data+3, "CE", 1);
  }

  if (0) {
    data[0] = 0xde;
    data[1] = 0xad;
    data[2] = 0xfa;
    data[3] = 0xce;

    char *sdata;
    sdata = printString(data, 4);
    cout << "printString0 ->" << sdata << "<-" << endl;
    cout << "printString1 ->" << printString(data, 4) << "<-" << endl;
  }

  if (0) {
    cout << "data ->";
    for (int i = 0; i < 4; ++i) cout << data[i];
    cout << "<-" << endl;
  }

  if (0) {
    cout << "A ->" << 'A'<< "<-" << endl;
    char A = 'A';
    cout << "char A ->" << A << "<-" << endl;
    char xA = 0x41;
    cout << "char xA ->" << xA << "<-" << endl;
    
    char nibbleH, nibbleL;
    data[0] = 0xDA;
    data[0] = 47;
    //    splitNibbles(data[0], nibbleH, nibbleL); 
    //    char nibbleP[3];
    char nibbleP[100];
    sprintf(nibbleP, "%x", data[0]);
    
    cout << "nibbleP     ->" << nibbleP << "<-" << endl;
    cout << "printString ->" << printString(data, 2) << "<-" << endl;
  }

  if (1) {
    blobData a = {1,
                  rd(), rd(), rd(),
                  rd(), rd(), rd(),
                  rd(), rd(), rd()
    };
    a.print();
    char *pa = a.serialize();
    cout << "serialized a   ->" << printString(pa, 16) << "<-" << endl;
    blobData b;
    b.deSerialize(pa); 
    b.print();
  }
  
  return 0;
}
