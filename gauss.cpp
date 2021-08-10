#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <cstdio>
#include <chrono>

using namespace std;

//from http://blog.ivank.net/fastest-gaussian-blur.html (algorithm 3)

void to_box(int box[], float sigma, int n)  
{
    float wIdeal = sqrt((12*sigma*sigma/n)+1); 
    int wl = floor(wIdeal); if(wl%2==0) wl--;
    int wu = wl+2;
                
    float mIdeal = (12*sigma*sigma - n*wl*wl - 4*n*wl - 3*n)/(-4*wl - 4);
    int m = round(mIdeal);
                
    for(int i=0; i<n; i++) box[i] = i < m ? wl : wu;
}

void H_Blur(float *& in, float *& out,int x,int y,int r)
{
	for(int i = 0 ; i < y ; i++){
	    	for(int j = 0 ; j < x ; j++){
	    	   float val=0;
	    	   for(int l = j-r ; l < j+r+1 ; l++){
	    	   	int xx = min(x-1, max(0,l));
	    	   	val += in[i*x + xx];
	    	   }
	    	   out[i*x + j] = val / (r+r+1);
	    	}
	}
}

void T_Blur(float *& in, float *& out,int x,int y,int r)
{
	for(int i = 0 ; i < y ; i++){
	    	for(int j = 0 ; j < x ; j++){
	    	   float val=0;
	    	   for(int k = i-r ; k < i+r+1 ; k++){
	    	   	int yy = min(y-1, max(0,k));
	    	   	val += in[yy*x + j];
	    	   }
	    	   out[i*x + j] = val / (r+r+1);
	    	}
	}
}

void box_blur(float *& in, float *& out, int x, int y, float sigma)
{
	swap(in , out);
	H_Blur(out, in, x, y, sigma);
	T_Blur(in, out, x, y, sigma);
}

void gauss_blur(float *& in, float *& out, int x, int y, float sigma) 
{
    int box[3];
    to_box(box, sigma, 3);
    
    box_blur(in, out, x, y, (box[0]-1)/2);
    box_blur(out, in, x, y, (box[1]-1)/2);
    box_blur(in, out, x, y, (box[2]-1)/2);
}

int main(int argc, char * argv[])
{
    
    if( argc < 2 ) exit(1);
    std::string filename = argv[1];
    const float sigma = argc > 2 ? atof(argv[2]) : 3.;
    
    int x,y,n;
    unsigned char *data = stbi_load(filename.c_str(), &x, &y, &n, 0);
    
    cout << "Image Properties : " << x << "x" << y << endl;
    
    if(data == nullptr)
    {
    	cout << "error while loading.	" << endl;
    	exit(1);
    }
    
    float * b_chanel = new float[x*y];
    float * g_chanel = new float[x*y];
    float * r_chanel = new float[x*y];
    float * new_b = new float[x*y];
    float * new_g = new float[x*y];
    float * new_r = new float[x*y];
    
    for(int i = 0; i < x*y; ++i)
    {
        b_chanel[i] = data[n*i + 0] / 255.f;
        g_chanel[i] = data[n*i + 1] / 255.f;
        r_chanel[i] = data[n*i + 2] / 255.f;
    }
    
    stbi_image_free(data);
    
    cout << "process... " << endl;
        
    auto start = chrono::system_clock::now();

    gauss_blur(b_chanel, new_b, x, y, sigma);
    gauss_blur(g_chanel, new_g, x, y, sigma);
    gauss_blur(r_chanel, new_r, x, y, sigma);
    
    auto end = chrono::system_clock::now();
    	
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end-start);
    
    cout << "time " << elapsed.count() << "ms" << endl;
    
    for(int i = 0; i < x*y; i++)
    {
        data[n * i + 0] = (unsigned char) min(255.f, max(0.f, 255.f * new_b[i]));
        data[n * i + 1] = (unsigned char) min(255.f, max(0.f, 255.f * new_g[i]));
        data[n * i + 2] = (unsigned char) min(255.f, max(0.f, 255.f * new_r[i]));
    }
    
    if(stbi_write_jpg("g_result.jpg", x, y, n, data, 100))
    	cout << "Done." << endl;
    else
    	cout << "error" << endl;
    
    delete[] b_chanel;
    delete[] g_chanel;
    delete[] r_chanel;
    delete[] new_r;
    delete[] new_b;
    delete[] new_g;
    
    return 0;
}
