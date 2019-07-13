#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>
using namespace std;

struct s{
	char x; // ASCII character
	int y; // stevilo pojavitev
	vector<bool> biti; // character v bitih
};
typedef struct s s;

struct s T[256] = {0};

class BinWriter {
public:
	int k;
	ofstream f;
	char x;

	BinWriter(const char *p) : k(0) {
		f.open(p, ios::binary);
	}

	~BinWriter() {
		if (k > 0) writeByte(x);
		f.close();
	}

	void writeByte(char x) {
		f.write((char*)&x, 1);
	}

	void writeInt(int y) {
		f.write((char*)&y, 4);
	}

	void writeBit(bool b) {
		if (k == 8) {
			writeByte(x);
			k = 0;
		}
		x ^= (-b ^ x) & (1 << k);
		k++;
	}

	void writeFloat(float z){
		f.write((char*)&z, 4);
	}
};

class BinReader {
public:
	int k;
	ifstream f;
	char x;
	int y;
	float z;

	BinReader(const char *p) : k(0) {
		f.open(p, ios::binary);
	}

	char readByte() {
		f.read((char*)&x, 1);
		return x;
	}

	bool readBit() {
		if (k == 8) {
			readByte();
			k = 0;
		}
		bool b = (x >> k) & 1;
		k++;
		return b;
	}

  	int readInt() {
		f.read((char*)&y, 4);
		return y;
	}

	float readFloat() {
		f.read((char*)&z, 4);
		return z;
	}

};

vector<char> readFile(string pot, int *L){
    ifstream vhodnaDatoteka;

    vhodnaDatoteka.open(pot, std::ifstream::binary);
    // če se datoteka ni uspešno odprla javimo napako
    if(!vhodnaDatoteka){
        cout << "Napaka pri odpiranju datoteke!\n" << endl;
        exit(1);
    }

		vhodnaDatoteka.seekg(0, std::ifstream::end);
		*L = vhodnaDatoteka.tellg();
		vhodnaDatoteka.seekg(0, ios::beg);

		vector<char> V(*L);

		vhodnaDatoteka.read( (char*)&V[0], *L);

		vhodnaDatoteka.close();

		return V;
}

int sortirnik(const void *prvi, const void *drugi){
	struct s *el1 = (struct s *)prvi;
	struct s *el2 = (struct s *)drugi;

	if( el1->y < el2->y)
		return 1;
	else if (el1->y > el2->y)
		return -1;
	else
		return 0;
}

void shannon_fano(int zacetniIndeks, int koncniIndeks, int nivo){
	//sestejemo frekvence
	if(zacetniIndeks==koncniIndeks-2) {

		T[zacetniIndeks].biti.push_back(false);
		T[koncniIndeks-1].biti.push_back(true);

		return;
	}
	if(zacetniIndeks==koncniIndeks-1) return;
	int meja = zacetniIndeks;
	int sprotniSestevek = 0;
	int sestevekFrekvenc = 0;
	for(int i=zacetniIndeks; i<koncniIndeks; i++)
		sestevekFrekvenc += T[i].y;
		
	//nastavimo vse bite nivoja na 0
	for(int i=zacetniIndeks; i<koncniIndeks; i++) {

		sprotniSestevek += T[i].y;

		if (sprotniSestevek<sestevekFrekvenc/2) {
		// 	printf("sprotni sestevek nivoja %d je %d, frekvence pa so %d \n", nivo, sprotniSestevek, sestevekFrekvenc);

			meja++;
		}else{
			meja++;
			break;
		}
	}
	int z;
	for(z=zacetniIndeks; z<meja; z++){
		T[z].biti.push_back(false);
		int a = T[z].x;
	}
	for(int i=meja; i<koncniIndeks; i++){
		T[i].biti.push_back(true);
	}
	// printf("zacetniIndeks: %i, koncniIndeks: %i, nivo: %i \n", zacetniIndeks, koncniIndeks, nivo);
	// if(zacetniIndeks == koncniIndeks) return;

	shannon_fano(zacetniIndeks, meja, nivo+1);
	shannon_fano(meja, koncniIndeks, nivo+1);


}

void kodiranje(char *vhodniArgument){

	int dolzinaVektorjaZnakov, steviloRazlicnihZnakov;
	vector<char> vektorZnakov;

	vektorZnakov = readFile(vhodniArgument, &dolzinaVektorjaZnakov);

	for(int i=0; i<256; i++){
		T[i].x = (char)i;
	}

	for(int i=0; i<dolzinaVektorjaZnakov; i++){
		unsigned char znak = vektorZnakov[i];
		T[(int)znak].y++;
	}

	qsort(T, 256, sizeof(struct s), &sortirnik);


	//prestejemo stevilo razlicnih znakov da nam ni treba zapisati celotne strukture in tako vemo koliko znakov bomo zapisali
	steviloRazlicnihZnakov=0;
	for( int i=0; i<256; i++ ){
		if(T[i].y == 0) break;
		steviloRazlicnihZnakov++;
	}

	shannon_fano(0, steviloRazlicnihZnakov, 0);

	BinWriter writer("out.bin");

	//zapisemo na prvo mesto stevilo razlicnih znakov
	writer.writeInt(steviloRazlicnihZnakov);

	//zapisemo vse znake, ki se pojavijo v besedilu
	for( int i=0; i<steviloRazlicnihZnakov; i++ ){
		writer.writeByte(T[i].x);
	}

	//zapisemo pojavitve vseh znakov
	for( int i=0; i < steviloRazlicnihZnakov; i++ ){
		writer.writeInt(T[i].y);
	}

	//sifriramo sporocilo
	for(int i=0; i < dolzinaVektorjaZnakov; i++){ //gremo skozi celo besedilo shranjeno v vektorju
		for(int j=0; j < steviloRazlicnihZnakov; j++){ //gremo skozi strukturo da lahko zakodiramo
			if( T[j].x == vektorZnakov[i] ){ //preverimo kateri znak je...
				for(int a=0; a<T[j].biti.size(); a++){ //in zapisemo vse njegove bite
					writer.writeBit( T[j].biti[a] ); //vzamemo a-ti bit vektorja "biti" ki se nahaja v strukturi T na mestu j
				}
			}
		}
	}
	float steviloVsehBitov = 0;
	int steviloBitov = 0;
	for(int j=0;j<steviloRazlicnihZnakov; j++){
		steviloVsehBitov += T[j].y * 8;
		steviloBitov += T[j].y * T[j].biti.size();
		for(int i=0; i<T[j].biti.size(); i++){
			cout << (int)T[j].biti[i];
		}
		cout << " : " << T[j].biti.size() << "b, " << T[j].y << ", " << T[j].x <<endl;
	}
	
	//dodam se 4byte za spremenljivko steviloRazlicnihZnakov, toliko bytov kolikor je znakov in toliko bytov kolikor zasedajo int-i za vsak znak
	steviloBitov+=8*(steviloRazlicnihZnakov+4*steviloRazlicnihZnakov)+4*8;
	printf("kompresijsko razmerje je: %f", steviloVsehBitov/steviloBitov);

	
	writer.f.close(); //zapremo datoteko
}

void dekodiranje(char *vhodniArgument){
	int steviloRazlicnihZnakov, sestevekBitov;
	BinReader br(vhodniArgument);
	BinWriter bw("out_d.bin");
	vector<bool> vektorPrebranihBitov;

	steviloRazlicnihZnakov = br.readInt();
	// pripravi strukturo
	for(size_t i = 0; i < 256; i++)
	{
		T[i].x='\0';
		T[i].y=0;
	}
	
	for(int i=0; i<steviloRazlicnihZnakov; i++)
		T[i].x = br.readByte(); 
	for(int i=0; i<steviloRazlicnihZnakov; i++)
		T[i].y = br.readInt();

	// struct s *Z = T;
	
	//zgradim drevo
	shannon_fano(0, 256, 0);

	for(int i = 0; i < steviloRazlicnihZnakov; i++)
	{
		sestevekBitov += T[i].biti.size() * T[i].y;
	}

	//dokler ne pridemo do znaka z najvec biti
	for(size_t i=0; i<sestevekBitov; i++){
		bool bit = br.readBit();
		vektorPrebranihBitov.push_back(bit);
		for(size_t j = 0; j < steviloRazlicnihZnakov; j++)
		{
			if(vektorPrebranihBitov == T[j].biti){
				// cout<<T[j].x;
				bw.writeByte(T[j].x);
				char tjx = T[j].x;
				vektorPrebranihBitov.clear();
			}
		}
	}
	bw.f.close();
	br.f.close();
}

int main(int argc, char **argv) {
	if(argc != 3) {
		printf("Uporaba programa:\n\t\t./vaja3 c alice.txt\n\t\t\tali\n\t\t./vaja3 d out.bin\n");
		exit(1);
	}

	if(strcmp(argv[1], "c") == 0)
		kodiranje(argv[2]);
	else if(strcmp(argv[1], "d") == 0)
		dekodiranje(argv[2]);
	else
		cout << "Prvi element je lahko le c ali d za kodiranje ali dekodiranje";



	return 0;
}
