#include <cstdio>
#include <map>
#include <vector>
#include <cstdlib>
using namespace std;
int main()
{
	//Shoot             125 188 8.0
	FILE *fp = fopen("Temp.txt", "w");
	map<int, vector<float>> mp;
	
	for (int i = 0; i < 60; ++i)
	{
		fprintf(fp, "1000 ShootFlameRand    39  60  8.0 0.0 22.5 2\n");
		fprintf(fp, "1000 ShootFlameRand    39  60  8.0 45.0 22.5 2\n");
		fprintf(fp, "1000 ShootFlameRand    39  60  8.0 90.0 22.5 2\n");
		fprintf(fp, "1000 ShootFlameRand    39  60  8.0 .0 22.5 2\n");
		fprintf(fp, "1000 ShootFlameRand    39  60  8.0 0.0 22.5 2\n");
		fprintf(fp, "1000 ShootFlameRand    39  60  8.0 0.0 22.5 2\n");
		fprintf(fp, "1000 ShootFlameRand    39  60  8.0 0.0 22.5 2\n");
		fprintf(fp, "1000 ShootFlameRand    39  60  8.0 0.0 22.5 2\n");
	} 
		
	
	
	/*
	for (const auto& p : mp)
	{
		fprintf(fp, "%-4d Shoot             250 88  8.0 ", p.first);
		for (const float& f : p.second)
			fprintf(fp, "%.1f ", f);
		fprintf(fp, "\n");
	}
	*/
	
	fclose(fp);
	return 0;
}
