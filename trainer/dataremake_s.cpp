#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctime>
using namespace std;

const int BSIZE=15;
const int BLSIZE=BSIZE*BSIZE;

int mm[BSIZE][BSIZE], moves[BSIZE*BSIZE];
float vals[BSIZE*BSIZE],svals[BSIZE*BSIZE],probs[BSIZE*BSIZE][BSIZE][BSIZE];

template <typename T>
void copyarr(T src[BSIZE][BSIZE], T dest[BSIZE][BSIZE])
{
	for (int i=0;i<BSIZE;i++)
		for (int j=0;j<BSIZE;j++)
			dest[i][j]=src[i][j];
}

template <typename T>
void rotate(T arr[BSIZE][BSIZE])
{
	T temp[BSIZE][BSIZE];
	copyarr(arr, temp);
	for (int i=0;i<BSIZE;i++)
		for (int j=0;j<BSIZE;j++)
			arr[i][j]=temp[j][BSIZE-i-1];
}

template <typename T>
void flip(T arr[BSIZE][BSIZE])
{
	T temp[BSIZE][BSIZE];
	copyarr(arr, temp);
	for (int i=0;i<BSIZE;i++)
		for (int j=0;j<BSIZE;j++)
			arr[i][j]=temp[i][BSIZE-j-1];
}

template <typename T>
void randomOp(T arr[BSIZE][BSIZE], int dir)
{
	if (dir>=4) flip(arr);
	for (int i=0;i<dir%4;i++)
		rotate(arr);
}

ofstream fout("data/gmkdata.txt", ios::app);

int nowcol; 

void printb()
{
	for (int i=0;i<BSIZE;i++,cout<<'\n')
		for (int j=0;j<BSIZE;j++)
			cout<<mm[i][j]<<' ';
}

int tmm[BSIZE][BSIZE];
float tprob[BSIZE][BSIZE];

void print(int step, int dir)
{
	copyarr<int>(mm,tmm);
	randomOp(tmm,dir);
	copyarr<float>(probs[step],tprob);
	randomOp(tprob,dir);
	
	for (int i=0;i<BSIZE;i++)
		for (int j=0;j<BSIZE;j++)
			if (tmm[i][j]==0)
				fout<<"0 "; 
			else if (tmm[i][j]==nowcol)
				fout<<"1 "; 
			else
				fout<<"0 "; 
	for (int i=0;i<BSIZE;i++)
		for (int j=0;j<BSIZE;j++)
			if (tmm[i][j]==0)
				fout<<"0 "; 
			else if (tmm[i][j]!=nowcol)
				fout<<"1 "; 
			else
				fout<<"0 ";
	
	for (int i=0;i<BSIZE * BSIZE;i++)
		fout<<tprob[i/BSIZE][i%BSIZE]<<" ";
}

void swap3(int arr[BSIZE][BSIZE])
{
	for (int i=0;i<BSIZE;i++)
		for (int j=0;j<BSIZE;j++)
			if (arr[i][j])
				arr[i][j]=arr[i][j]%2+1;
}

void calcVal(int nums, int val0)
{
	float first_z=0.6f;
	float decay=pow(first_z, 1.0f / nums);
	float alpha=0.4f;
	float mcdecay=0.65f;
	vals[nums-1]=val0;
	//svals[nums-1]/=mcdecay; bug
	for (int i=nums-2;i>=0;i--)
	{
		vals[i]=vals[i+1]*decay; //decay ?
		svals[i]=(1 - mcdecay) * svals[i] - svals[i+1] * mcdecay;
	}
			
	for (int i=nums-1;i>=0;i--)
		if (i&1) vals[i]=-vals[i];
		
	for (int i=nums-2;i>=0;i--)
		vals[i]=alpha * vals[i] + (1-alpha) * svals[i];
}



int main(int argc, char **argv)
{
	srand((unsigned)time(NULL));
	cout<<"gathering data form "<<argv[1]<<'\n';
	ifstream fin(argv[1]);
	int num, vcnt=0, gamecnt=0, win1=0;
	while (fin>>num)
	{
		nowcol=1;
		memset(mm,0,sizeof(mm));
		for (int i=0;i<num;i++)
		{
			fin>>moves[i];
			fin>>svals[i];
			for (int j=0;j<BLSIZE;j++)
				fin>>probs[i][j/BSIZE][j%BSIZE];
		}
		int val; fin>>val;
		if (val==2)
			val=-1;
		if (val==1) win1++;
		calcVal(num, val);
		for (int i=0;i<num;i++)
		{
			if (moves[i]!=BLSIZE)
			{ 
				print(i, 0);
				fout<<vals[i]<<'\n';
				cout<<vals[i]<<' ';
				print(i, rand()%8);
				fout<<vals[i]<<'\n';
			}
			if (moves[i]==BLSIZE)
				swap3(mm);
			else
				mm[moves[i]/BSIZE][moves[i]%BSIZE]=nowcol;
			nowcol=nowcol%2+1;
		}
		vcnt+=num;
		gamecnt++;
		if (gamecnt%50==0) cout<<gamecnt<<'\n';
		if (gamecnt==1200) break;
	}
	cout<<vcnt<<" boards\n"<<gamecnt<<" games\n"<<win1<<" black wins\n";
	return 0;
}

