#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>


/*
 * Description :   main file that uses HashTable.
 *
 * USAGE: ./exe <commandFile>
 * example: ./exe tests/2inp
 *
 * Author(s)   :   Sonal Ranjan
 * Status      :   under development
 */

#include "hash.h"

/*=************************************************
 * Test harness to be used if no command file is given 
 * **************************************************=*/
void TestHarness1() 
{
    SR::HashTable<SR::KeyConstChar, SR::VLong> hf;
    hf.Put("Hello Dolly", 1);
    hf.Put("Hello Willy", 4);
    long i = 0;
    long st = hf.Get((const char*)"Hello Willy", i);
    cout << " Willy: " << i << endl;
    hf.Dump();
    return;
}

/*=************************************************
 * primitive parser for command file
 * **************************************************=*/
void CommandParse( std::istream& inpf)
{
    char buf[4096]; 
    long val ;
    long hasKey = 0;
    SR::HashTable<SR::KeyConstChar, SR::VLong> hf;
    inpf >> buf;
    while (!inpf.eof()) {
	switch(toupper(buf[0])) {
	    case 'P': // put
		inpf >> buf;
		inpf >> val;
		printf("CMD: P(%s, %ld) \n", buf, val);
		hf.Put( strdup(buf), val);
		break;
	    case 'G': // get
		inpf >> buf;
		printf("CMD: G(%s) => (v=%ld, %d)\n", buf, val, hasKey); 
		hasKey = hf.Get( buf, val);
		break;
	    case 'R':
		inpf >> buf;
		printf("CMD: R(%s) \n", buf);
		hf.Remove( buf);
		break;
	    case 'D': // dump
		printf("CMD: D \n");
		hf.Dump();
		break;
	    case 'C':
		printf("CMD: C \n");
		hf.Compact();
		break;
	    case 'T': // dump
		inpf >> (std::hex) >> val >> (std::dec);
		printf("CMD: T=%ld \n", val);
		hf.SetTraceLevel(val);
		break;
	    default:
		break;
	}
	inpf >> buf;
    }
}


/*=************************************************
 * Run a command file, if provided - else run default harness
 * **************************************************=*/
int
main(int argc , char* argv[])
{
    std::ifstream cmdFile;
    if ((argc > 1) && (cmdFile.open(argv[1]), cmdFile)) { 
	CommandParse( cmdFile);
    } else {
	TestHarness1();
    }
}

