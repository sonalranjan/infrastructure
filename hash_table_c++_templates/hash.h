#ifndef SR_HASH_H
#define	SR_HASH_H

/*
 * Description :   Implementation of HashTable (with Get, Put, Remove
 * APIs) 
 * Author(s)   :   Sonal Ranjan (sonal.ranjan@gmail.com)
 * Status      :   under development, some testing done
 */

#include <iostream>
#include <iomanip>

using std::cout;
using std::ostream;
using std::endl;
using std::setw;
#define	myAssert assert

namespace SR {

    typedef const char* KeyConstChar;
    typedef long VLong;

    template<typename T>
    class CmpFn {
	public:
	    int operator()(T& qs1, T& qs2) { return (qs1 - qs2); }
    };

    // specialization for string
    template<>
    class CmpFn<const char*> {
	public:
	    int operator()(const char* qs1, const char* qs2) { 
		return strcmp(qs1, qs2); 
	    }
    };

    template<typename T>
    class HashFn {
	public:
	    long operator()(T& qs, int hint=0) { return hint; }
    };

    // specialization for string
    template<>
    class HashFn<const char*> {
	public:
	    long operator()(const char* qs, int hint=0) {
		int shift1 = (hint? 4:5);
		int shift2 = (hint? 5:4);
		unsigned int seed=0;
		for( seed = 0; *qs; 
			(seed = (seed<<shift1) - seed + (seed>>shift2) 
			 + (0xdf & *qs)), ++qs);
		return seed;
	    }
    };


    
/**
 * HashTable<KeyType, ValueType> 
 * 
 * First cut implementation for HashTable. 
 * Uses closed-hashing with 2-level hash functions. 
 * @warning	assumes user provides storage for keys
 */
    template<typename K, typename V, 
	typename KHashFn = HashFn<K>, typename KCmpFn = CmpFn<K> >
    class HashTable {

	protected:
	    // this will contain <Val, strIndex>
	    class Pair {
		public:
		    V val;
		    long keyIdx;
		    Pair(V v=0, long k=0) : 
			val(v), keyIdx( k) {}
	    };

	    long trace;
	    long liveCount;
	    long count;
	    long size;
	    double effortPerOp;
	    K* keyArr;
	    Pair* hashArr;
	    KHashFn hashFn;
	    KCmpFn cmpFn;

	public:
	    long Count() { return liveCount; }
	    void SetTraceLevel(long tl) { trace = tl; } 
	    int Compact() { return Resize(); }

	protected:
	    bool needCompaction() { 
		if (count <= 4) return false; 
		return ( ((float)liveCount/count) < 0.6); 
	    }

	    long getSuggestedResize() { 
		if ( (liveCount+5) > (4.0*size/5.0)) { return size*2; }
		if (size <= 16) return size;
		if (size/2.0 > liveCount) { return size/2; }
		return size;
	    }


	public:
	    HashTable() : 
		trace(0), liveCount(0), count(1), 
		size(8), effortPerOp(0) {
		    keyArr = new K[size]; hashArr = new Pair[size]; 
		    memset(keyArr, 0, sizeof(K)*size);
		}

	    ~HashTable() {
		delete [] keyArr; delete [] hashArr;
	    }

	    /*=************************************************
	     * Get: look for Key qs, return bool hasKey. 
	     * (v is INOUT parameter)
	     * **************************************************=*/
	    int Get(K qs, V& v) {
		long hashIndex=-1;
		long keyIdx = Locate(qs, hashIndex, false);
		if (trace) 
		    printf("TRACE: Get: qs=%s, hashIndex=%ld keyIdx=%ld\n",
			    qs, hashIndex, keyIdx);
		if (hashIndex >= 0) {
		    v = hashArr[hashIndex].val;
		    return 0;
		}
		return -1;
	    }

	    /*=************************************************
	     * Put: insert value v for key qs
	     * **************************************************=*/
	    int Put(K qs, V v) {
		Resize();
		long hashIndex=-1;
		long keyIdx = Locate(qs, hashIndex, false);
		if (trace)
		    printf("TRACE: Put: qs=%s, hashIndex=%ld keyIdx=%ld\n", 
			    qs, hashIndex, keyIdx);
		if (hashIndex >= 0) {
		    hashArr[hashIndex].val = v;
		    if (keyIdx <= 0) {
			hashArr[hashIndex].keyIdx = count;
			keyArr[ count ] = qs;
			count++;
		    } else {
			/* reuse of some earlier killed string's location */
			hashArr[hashIndex].keyIdx = keyIdx;
			keyArr[ keyIdx ] = qs;
		    }
		    liveCount++;
		    return 0;
		}
		return -1;
	    }

	    /*=************************************************
	     * Remove: the value associated with key qs. do nothing if
	     * the key does not exist. (return value indicates success
	     * or failure in finding a key in HashTable)
	     * **************************************************=*/
	    int Remove(K qs) {
		long hashIndex=-1;
		long keyIdx = Locate(qs, hashIndex, false);
		/* valid hashArr location, with a live string (an
		 * already killed string will have keyIdx < 0) */
		if (hashIndex >= 0 && keyIdx > 0) {
		    keyArr[ hashArr[hashIndex].keyIdx ] = 0;
		    hashArr[hashIndex].keyIdx *= -1;
		    liveCount--;
		    return 0;
		}
		return -1;
	    }

	    /*=************************************************
	     * Print routine
	     * **************************************************=*/
	    void Dump(bool detail = true, ostream& os = std::cout) {
		/* print a summary */
		char p = os.fill('*');
		os <<  setw(40) << " HashTable " << setw(20) << "" << endl;
		os.fill(p);
		os << "HashTable Summary: liveCount=" << liveCount 
	   	   << " Count=" << count 
		   << " Size=" << size ;
		if (effortPerOp > 0) {
		    os << "HashTable: Effort per op=" << effortPerOp;
		}
		os << endl;

		if (!detail) {
		    p = os.fill('*'); os << setw(60) << "" << endl; os.fill(p);
		    return;
		}
		p = os.fill('-'); os << setw(60) << "" << endl; os.fill(p);

		/* print the valid entries if details asked for */
		if (liveCount) {
		    os << setw(15) << "HashIndex" 
		    << setw(15) << "Value" 
		    << setw(15) << "keyIndex" 
		    << " String" << endl;
		}
		for (int i=0; i < size; i++) {
		    if (hashArr[i].keyIdx == 0) continue;
		    os << setw(15) << i
		    << setw(15) << hashArr[i].val 
		    << setw(15) << hashArr[i].keyIdx;
		    if (hashArr[i].keyIdx > 0) { 
			os << " " << keyArr[ hashArr[i].keyIdx];
		    }
		    os << endl;
		}
		p = os.fill('*'); os << setw(60) << "" << endl; os.fill(p);

	    }

	protected:

	    /*=************************************************
	     * Resize: expand or compact as the need be
	     * **************************************************=*/
	    bool Resize() {
		long newSize = size;
		newSize = getSuggestedResize();
		if ((newSize == size) && !needCompaction()) {
		    // do not resize or compact if not necessary
		    return false; 
		}

		if (trace) 
		    printf("TRACE: Resize: (size=%ld, newSize=%ld), needCompact=%d\n",
			size, newSize, needCompaction());

		//------------------------------------------------------
		// 1. copy old values
		// 2. if needed realloc hashArr/keyArr
		// 3. compact and reHash
		//------------------------------------------------------
		K* oldKeyArr = keyArr;
		V tempVals[count]; // should be same as alloca()

		/* 1. copying values */
		for (int i=0; i < size; i++) {
		    if ( hashArr[i].keyIdx > 0) {
			tempVals[ hashArr[i].keyIdx ] = hashArr[i].val;
		    }
		}

		/* 2. reallocate and/or reinit */
		if (size != newSize) {
		    size = newSize;
		    delete [] hashArr;
		    hashArr = new Pair[size]; keyArr = new K[size];
		    memset(keyArr, 0, sizeof(K)*size);
		} else {
		    memset( hashArr, 0, sizeof(Pair)*size);
		}

		/* 3. compaction and rehashing */
		long newIdx = 1, pairIndex = 0;
		for (int i=1; i<count; i++) {
		    if (0 == oldKeyArr[i]) continue;
		    // copy string 
		    keyArr[newIdx]=oldKeyArr[i];
		    // acquire new hashIndex
		    Locate( keyArr[newIdx], pairIndex, /*rehash=*/true);
		    // copy string index and val to hashIndex
		    hashArr[pairIndex] = Pair(tempVals[i],newIdx);
		    newIdx++;
		}
		if (oldKeyArr != keyArr) { delete [] oldKeyArr; }
		count = liveCount+1;
		myAssert( count == newIdx);
		return true;
	    }

	    /*=************************************************
	     * Locate: find the key qs in HashTable. Or find a bucket
	     * where qs can be inserted. return value is an index into
	     * the keyArr when valid (non-zero). INOUT index is the
	     * bucket index for looking-up/inserting value.
	     * **************************************************=*/
	    long Locate(K qs, long& index, bool rehash) {
		long h = hashFn( qs, 0);
		index = h & (size-1);
		Pair& p = hashArr[index];
		if (trace) 
		    printf("TRACE: qs=%s, h=%ld h1=%ld\n", qs, h, index);

		// there is no value with this hash
		if (p.keyIdx == 0) return -1;
		if (!rehash) {
		    // -ve index: there was a key here that has been killed
		    // now reuse the index
		    if (p.keyIdx < 0) { return (-p.keyIdx); }
		    // +ve index: does this string-index match the key ?
		    if (cmpFn(keyArr[p.keyIdx], qs) == 0) {
			return p.keyIdx;
		    }
		}
		// in case of rehash, non-zero index means -> keep
		// looking further: first level collision

		long g = hashFn( qs, h) | 0x1 ; 
		// size and g are mutually-prime
		if (trace) printf("TRACE: qs=%s, g=%ld \n", qs, g);
		for (int i=0; i < count; i++) {
		    h += g;
		    index = h & (size-1);
		    Pair& p = hashArr[index];

		    // this bucket is empty
		    if (p.keyIdx == 0) return -1;
		    if (!rehash) {
			/* same logic as above */
			if (p.keyIdx < 0) { return (-p.keyIdx); }
			if (cmpFn(keyArr[p.keyIdx], qs) == 0) {
			    return p.keyIdx;
			}
		    } 
		    // in case of rehash, non-zero index means -> keep
		    // looking further
		}

		myAssert(0);
		/* should never reach here */
		index = -1; 
		return -1;
	    }

    };
}

#endif

