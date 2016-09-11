
#include <stdio.h>

typedef unsigned short word;
typedef unsigned int   uint;
typedef unsigned char  byte;

uint flen( FILE* f ) {
  fseek( f, 0, SEEK_END );
  uint len = ftell(f);
  fseek( f, 0, SEEK_SET );
  return len;
}

void* fload( FILE* temp, uint& f_len /*char* fname*/ ) {
//  FILE* temp = fopen(fname,"rb");
//  if (temp==0) return 0;
  unsigned int len = flen(temp);
  char* buf = new char[len]; if( buf ) len = fread( buf, 1,len, temp );
//  fclose( temp );
  f_len = len;
  return buf;
}

void fsave( FILE* g, /*char* fname,*/ void* buf, uint len ) {
//  FILE* g = fopen( fname, "wb" );
//  if( g ) {
    fwrite( buf, 1,len, g );
//    fclose(g);
//  }
}


volatile uint uneG = 'uneG';
volatile uint Ieni = 'Ieni';
volatile uint letn = 'letn';

int main( int argc, char** argv ) {
  if( argc<2 ) {
    printf( "Usage: ic64patch file.exe [patched_file]\n" );
    return 1;
  }

  FILE* f = fopen( argv[1], "r+b" ); if( f==0 ) {
    printf( "Cannot open \"%s\"\n", argv[1] );
    return 2;
  }

  FILE* g = f; char* gnam=argv[1];

  if( argc>=3 ) {
    g = fopen( argv[2], "wb" ); if( g==0 ) {
      printf( "Cannot open \"%s\"\n", argv[2] );
      return 3;
    } else gnam = argv[2];
  }
  
  printf( "Loading \"%s\": ", argv[1] );

  uint f_len = 0;
  byte* p = (byte*)fload( f, f_len ); if( p==0 ) {
    printf( "Couldn't load the file to memory\n" );
    return 4;
  }

  printf( "%i bytes\n", f_len );

  // 1. cmp xxx,"Genu" checks
/*
817C242447656E75               cmp         d,[rsp][024],0756E6547 ;'uneG'
75D9                           jnz        .00000001`4001F811 --1
817C242C696E6549               cmp         d,[rsp][02C],049656E69 ;'Ieni'
75CF                           jnz        .00000001`4001F811 --1
817C24286E74656C               cmp         d,[rsp][028],06C65746E ;'letn'
75C5                           jnz        .00000001`4001F811 --1

40027CE8: B800000000                     mov         eax,0
40027CED: 0FA2                           cpuid
40027CEF: 399D70010000                   cmp         [rbp][000000170],ebx
40027CF5: 751D                           jnz        .00000001`40027D14 --
40027CF7: 399574010000                   cmp         [rbp][000000174],edx
40027CFD: 7515                           jnz        .00000001`40027D14 --
40027CFF: 398D78010000                   cmp         [rbp][000000178],ecx
40027D05: 750D                           jnz        .00000001`40027D14 --

40C6BA: B800000000                     mov         eax,0
40C6BF: 0FA2                           cpuid
40C6C1: 395DC0                         cmp         [ebp][-040],ebx
40C6C4: 751C                           jnz        .00040C6E2 --6
40C6C6: 3955C4                         cmp         [ebp][-03C],edx
40C6C9: 7517                           jnz        .00040C6E2 --6
40C6CB: 394DC8                         cmp         [ebp][-038],ecx
40C6CE: 7512                           jnz        .00040C6E2 --6
*/

  int i;
  uint p_intel = 0;
  uint flag = 0;

  for( i=5; i<int(f_len)-64; i++ ) {

    if( ((uint&)p[i]==0x000000B8) && ((uint&)p[i+4]==0x39A20F00) && (p[i+13]==0x75) && (p[i+13+1]!=0) && (p[i+13+8]==0x75) && (p[i+13+16]==0x75) ) {
      printf( "xmm0 check for \"GenuineIntel\" patched at %X\n", i );
      p[i+13+1]=0; p[i+13+8+1]=0; p[i+13+16+1]=0; flag=1;
    }

    if( ((uint&)p[i]==0x000000B8) && ((uint&)p[i+4]==0x39A20F00) && (p[i+10]==0x75) && (p[i+10+1]!=0) && (p[i+10+5]==0x75) && (p[i+10+10]==0x75) ) {
      printf( "indirect check for \"GenuineIntel\" patched at %X\n", i );
      p[i+10+1]=0; p[i+10+5+1]=0; p[i+10+10+1]=0; flag=1;
    }

    if( (((uint&)p[i])==uneG) && (((uint&)p[i+4])==Ieni) && (((uint&)p[i+8])==letn) && ((uint&)p[i+12]==0) ) {
      printf( "\"GenuineIntel\" string patched at %X\n", i ); p_intel=i;
      (uint&)p[i]=0; (uint&)p[i+4]=0; (uint&)p[i+8]=0; flag=1;
    }

    if( ((uint&)p[i]=='htuA') && ((uint&)p[i+4]=='itne') && ((uint&)p[i+8]=='DMAc') && ((uint&)p[i+12]==0) ) {
      printf( "\"AuthenticAMD\" string patched at %X\n", i ); 
      (uint&)p[i]=0; (uint&)p[i+4]=0; (uint&)p[i+8]=0; flag=1;
    }

    if( (((uint&)p[i])==uneG) && ((p[i+4]==0x75) || ((word&)p[i+4]==0x850F)) ) {
      printf( "\"Genu\" patched at %X\n", i ); 
      (uint&)p[i]=0; p[i+5]=0; flag=1;
    }

    if( (((uint&)p[i])==Ieni) && ((p[i+4]==0x75) || ((word&)p[i+4]==0x850F)) ) {
      printf( "\"Ieni\" patched at %X\n", i ); 
      (uint&)p[i]=0; p[i+5]=0; flag=1;
    }

    if( (((uint&)p[i])==letn) && ((p[i+4]==0x75) || ((word&)p[i+4]==0x850F)) ) {
      printf( "\"letn\" patched at %X\n", i ); 
      (uint&)p[i]=0; p[i+5]=0; flag=1;
    }

  }

  if( flag ) {
    printf( "Writing the patched file to \"%s\"\n", gnam );
    fseek( g, 0, SEEK_SET );
    fsave( g, p, f_len );
  } else {
    printf( "Nothing is modified, no need to write the file\n" );
  }

}






