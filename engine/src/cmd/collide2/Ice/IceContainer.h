/*
 * IceContainer.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a simple container class.
 *	\file		IceContainer.h
 *	\author		Pierre Terdiman
 *	\date		February, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2023-05-27
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_CONTAINER_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_CONTAINER_H

#define CONTAINER_STATS

enum FindMode {
    FIND_CLAMP,
    FIND_WRAP,

    FIND_FORCE_DWORD = 0x7fffffff
};

class ICECORE_API Container {
public:
    // Constructor / Destructor
    Container();
    Container(const Container &object);
    Container(uint32_t size, float growth_factor);
    ~Container();
    // Management
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	A O(1) method to add a value in the container. The container is automatically resized if needed.
     *	The method is inline, not the resize. The call overhead happens on resizes only, which is not a problem since the resizing operation
     *	costs a lot more than the call overhead...
     *
     *	\param		entry		[in] a uint32_t to store in the container
     *	\see		Add(float entry)
     *	\see		Empty()
     *	\see		Contains(uint32_t entry)
     *	\return		Self-Reference
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_    Container &Add(uint32_t entry) {
        // Resize if needed
        if (mCurNbEntries == mMaxNbEntries) {
            Resize();
        }

        // Add new entry
        mEntries[mCurNbEntries++] = entry;
        return *this;
    }

    inline_    Container &Add(const uint32_t *entries, uint32_t nb) {
        // Resize if needed
        if (mCurNbEntries + nb > mMaxNbEntries) {
            Resize(nb);
        }

        // Add new entry
        CopyMemory(&mEntries[mCurNbEntries], entries, nb * sizeof(uint32_t));
        mCurNbEntries += nb;
        return *this;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	A O(1) method to add a value in the container. The container is automatically resized if needed.
     *	The method is inline, not the resize. The call overhead happens on resizes only, which is not a problem since the resizing operation
     *	costs a lot more than the call overhead...
     *
     *	\param		entry		[in] a float to store in the container
     *	\see		Add(uint32_t entry)
     *	\see		Empty()
     *	\see		Contains(uint32_t entry)
     *	\return		Self-Reference
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_    Container &Add(float entry) {
        // Resize if needed
        if (mCurNbEntries == mMaxNbEntries) {
            Resize();
        }

        // Add new entry
        mEntries[mCurNbEntries++] = IR(entry);
        return *this;
    }

    inline_    Container &Add(const float *entries, uint32_t nb) {
        // Resize if needed
        if (mCurNbEntries + nb > mMaxNbEntries) {
            Resize(nb);
        }

        // Add new entry
        CopyMemory(&mEntries[mCurNbEntries], entries, nb * sizeof(float));
        mCurNbEntries += nb;
        return *this;
    }

    //! Add unique [slow]
    inline_    Container &AddUnique(uint32_t entry) {
        if (!Contains(entry)) {
            Add(entry);
        }
        return *this;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Clears the container. All stored values are deleted, and it frees used ram.
     *	\see		Reset()
     *	\return		Self-Reference
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Container &Empty();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Resets the container. Stored values are discarded but the buffer is kept so that further calls don't need resizing again.
     *	That's a kind of temporal coherence.
     *	\see		Empty()
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_    void Reset() {
        // Avoid the write if possible
        // ### CMOV
        if (mCurNbEntries) {
            mCurNbEntries = 0;
        }
    }

    // HANDLE WITH CARE
    inline_    void ForceSize(uint32_t size) {
        mCurNbEntries = size;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Sets the initial size of the container. If it already contains something, it's discarded.
     *	\param		nb		[in] Number of entries
     *	\return		true if success
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool SetSize(uint32_t nb);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Refits the container and get rid of unused bytes.
     *	\return		true if success
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Refit();

    // Checks whether the container already contains a given value.
    bool Contains(uint32_t entry, uint32_t *location = nullptr) const;
    // Deletes an entry - doesn't preserve insertion order.
    bool Delete(uint32_t entry);
    // Deletes an entry - does preserve insertion order.
    bool DeleteKeepingOrder(uint32_t entry);
    //! Deletes the very last entry.
    inline_    void DeleteLastEntry() {
        if (mCurNbEntries) {
            mCurNbEntries--;
        }
    }
    //! Deletes the entry whose index is given
    inline_    void DeleteIndex(uint32_t index) {
        mEntries[index] = mEntries[--mCurNbEntries];
    }

    // Helpers
    Container &FindNext(uint32_t &entry, FindMode find_mode = FIND_CLAMP);
    Container &FindPrev(uint32_t &entry, FindMode find_mode = FIND_CLAMP);
    // Data access.
    inline_    uint32_t GetNbEntries() const {
        return mCurNbEntries;
    }    //!< Returns the current number of entries.
    inline_    uint32_t GetEntry(uint32_t i) const {
        return mEntries[i];
    }    //!< Returns ith entry
    inline_    uint32_t *GetEntries() const {
        return mEntries;
    }    //!< Returns the list of entries.

    inline_    uint32_t GetFirst() const {
        return mEntries[0];
    }

    inline_    uint32_t GetLast() const {
        return mEntries[mCurNbEntries - 1];
    }

    // Growth control
    inline_    float GetGrowthFactor() const {
        return mGrowthFactor;
    }    //!< Returns the growth factor
    inline_    void SetGrowthFactor(float growth) {
        mGrowthFactor = growth;
    }    //!< Sets the growth factor
    inline_    bool IsFull() const {
        return mCurNbEntries == mMaxNbEntries;
    }    //!< Checks the container is full
    inline_    BOOL IsNotEmpty() const {
        return mCurNbEntries;
    }    //!< Checks the container is empty

    //! Read-access as an array
    inline_    uint32_t operator[](uint32_t i) const {
        OPASSERT(i >= 0 && i < mCurNbEntries);
        return mEntries[i];
    }
    //! Write-access as an array
    inline_    uint32_t &operator[](uint32_t i) {
        OPASSERT(i >= 0 && i < mCurNbEntries);
        return mEntries[i];
    }

    // Stats
    uint32_t GetUsedRam() const;

    //! Operator for "Container A = Container B"
    void operator=(const Container &object);

#ifdef CONTAINER_STATS

    inline_    uint32_t GetNbContainers() const {
        return mNbContainers;
    }

    inline_    uint32_t GetTotalBytes() const {
        return mUsedRam;
    }

private:

    static uint32_t mNbContainers;        //!< Number of containers around
    static uint32_t mUsedRam;            //!< Amount of bytes used by containers in the system
#endif
private:
    // Resizing
    bool Resize(uint32_t needed = 1);
    // Data
    uint32_t mMaxNbEntries;        //!< Maximum possible number of entries
    uint32_t mCurNbEntries;        //!< Current number of entries
    uint32_t *mEntries;            //!< List of entries
    float mGrowthFactor;        //!< Resize: new number of entries = old number * mGrowthFactor
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_CONTAINER_H
