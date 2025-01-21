///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains all memory macros.
 *	\file		IceMemoryMacros.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 *
 *  Copyright (C) 2000-2025 Pierre Terdiman, Stephen G. Tuggy, Benjamen R. Meyer
 *  Public Domain
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_MEMORY_MACROS_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_MEMORY_MACROS_H

#undef ZeroMemory
#undef CopyMemory
#undef MoveMemory
#undef FillMemory

//!	Clears a buffer.
//!	\param		addr	[in] buffer address
//!	\param		size	[in] buffer length
//!	\see		FillMemory
//!	\see		StoreDwords
//!	\see		CopyMemory
//!	\see		MoveMemory
inline_ void ZeroMemory(void *addr, size_t size) {
    memset(addr, 0, size);
}

//!	Fills a buffer with a given byte.
//!	\param		addr	[in] buffer address
//!	\param		size	[in] buffer length
//!	\param		val		[in] the byte value
//!	\see		StoreDwords
//!	\see		ZeroMemory
//!	\see		CopyMemory
//!	\see		MoveMemory
inline_ void FillMemory(void *dest, size_t size, uint8_t val) {
    memset(dest, val, size);
}

//!	Fills a buffer with a given dword.
//!	\param		addr	[in] buffer address
//!	\param		nb		[in] number of dwords to write
//!	\param		value	[in] the dword value
//!	\see		FillMemory
//!	\see		ZeroMemory
//!	\see		CopyMemory
//!	\see		MoveMemory
//!	\warning	writes nb*4 bytes !
inline_ void StoreDwords(uint32_t *dest, uint32_t nb, uint32_t value) {
    // The asm code below **SHOULD** be equivalent to one of those C versions
    // or the other if your compiled is good: (checked on VC++ 6.0)
    //
    //	1) while(nb--)	*dest++ = value;
    //
    //	2) for(uint32_t i=0;i<nb;i++)	dest[i] = value;
    //
    /*_asm push eax
    _asm push ecx
    _asm push edi
    _asm mov edi, dest
    _asm mov ecx, nb
    _asm mov eax, value
    _asm rep stosd
    _asm pop edi
    _asm pop ecx
    _asm pop eax*/
    while (nb--) {
        *dest++ = value;
    }
}

//!	Copies a buffer.
//!	\param		addr	[in] destination buffer address
//!	\param		addr	[in] source buffer address
//!	\param		size	[in] buffer length
//!	\see		ZeroMemory
//!	\see		FillMemory
//!	\see		StoreDwords
//!	\see		MoveMemory
inline_ void CopyMemory(void *dest, const void *src, size_t size) {
    memcpy(dest, src, size);
}

//!	Moves a buffer.
//!	\param		addr	[in] destination buffer address
//!	\param		addr	[in] source buffer address
//!	\param		size	[in] buffer length
//!	\see		ZeroMemory
//!	\see		FillMemory
//!	\see		StoreDwords
//!	\see		CopyMemory
inline_ void MoveMemory(void *dest, const void *src, size_t size) {
    memmove(dest, src, size);
}

#define SIZEOFOBJECT        sizeof(*this)                                    //!< Gives the size of current object. Avoid some mistakes (e.g. "sizeof(this)").
//#define CLEAROBJECT		{ memset(this, 0, SIZEOFOBJECT);	}			//!< Clears current object. Laziness is my business. HANDLE WITH CARE.

template<typename T>
inline void DELETESINGLE(T *&x) {                                             //!< Deletes an instance of a class.
    if (x) {
        delete x;
        x = nullptr;
    }
}

template<typename T>
inline void DELETEARRAY(T *&x) {                                              //!< Deletes an array.
    if (x) {
        delete[]x;
        x = nullptr;
    }
}

template<typename T>
inline void SAFE_RELEASE(T *&x) {                                             //!< Safe D3D-style release
    if (x) {
        x->Release();
        x = nullptr;
    }
}

template<typename T>
inline void SAFE_DESTRUCT(T *&x) {                                            //!< Safe ICE-style release
    if (x) {
        x->SelfDestruct();
        x = nullptr;
    }
}

#ifdef __ICEERROR_H__
#define CHECKALLOC(x)		if(!x) return SetIceError("Out of memory.", EC_OUT_OF_MEMORY);	//!< Standard alloc checking. HANDLE WITH CARE.
#else
#define CHECKALLOC(x)        if(!x) return false;
#endif

//! Standard allocation cycle
#define SAFE_ALLOC(ptr, type, count)    DELETEARRAY(ptr);    ptr = new type[count];    CHECKALLOC(ptr);

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_MEMORY_MACROS_H
