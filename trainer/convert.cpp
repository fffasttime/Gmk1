#include <iostream>
#include <cstdio>
using namespace std;
const int BSIZE=15;
const int BLSIZE=BSIZE*BSIZE;

float policy[BLSIZE][BLSIZE],vals[BLSIZE];int moves[BLSIZE];
int convert(int argc, char **argv){
	FILE *in=fopen(argv[1],"r");
	FILE *out;
	if (argc==3) out=fopen(argv[2],"wb");
	int n;
	int gcnt=0;
	while (~fscanf(in,"%d",&n)){
		fwrite(&n,4,1,out);
		for (int i=0;i<n;i++){
			fscanf(in,"%d%f",moves+i,vals+i);
			for (int j=0;j<BLSIZE;j++)
				fscanf(in,"%f",&policy[i][j]);
		}
		int z; fscanf(in,"%d",&z);
		fwrite(&z,4,1,out);
		for (int i=0;i<n;i++){
			fwrite(moves+i,4,1,out);
			fwrite(vals+i,4,1,out);
			for (int j=0;j<BLSIZE;j++)
				fwrite(policy[i]+j,4,1,out);
		}
		gcnt++;
		if (gcnt%100==0) cout<<gcnt<<'\n';
	}
	return 0;
}

int read(){
	FILE *in=fopen("selfplaydata.bin","rb");
	int n;
	fread(&n,4,1,in);
	int z; 
	fread(&z,4,1,in);
	int i=1;
	fread(&moves[i],4,1,in);
	fread(&vals[i],4,1,in);
	cout<<moves[i]<<' '<<vals[i]<<'\n';
	for (int j=0;j<BLSIZE;j++){
		fread(&policy[i][j],4,1,in);
		cout<<policy[i][j]<<' ';
	}
	return 0;
}

int main(int argc, char **argv){
	convert(argc,argv);
	return 0;
}
