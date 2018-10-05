// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2018-2018 The VERGE Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERGE_PRIMITIVES_BLOCK_H
#define VERGE_PRIMITIVES_BLOCK_H

#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>

enum
{
    ALGO_SCRYPT  = 0,
    ALGO_X17	 = 1,
    ALGO_LYRA2RE = 2,
    ALGO_BLAKE   = 3,
    ALGO_GROESTL = 4,
    NUM_ALGOS
};

enum
{
    // primary version
    BLOCK_VERSION_DEFAULT        = 2,
	//BLOCK_VERSION_STEALTH        = 4,

    // algo
    BLOCK_VERSION_ALGO_BROKEN    = (10 << 11), //'101000000000000' 10 (broken bitmask)
    BLOCK_VERSION_ALGO           = (15 << 11), //'111100000000000' 15 (bitmask)
    BLOCK_VERSION_SCRYPT         = (1  << 11), //'000100000000000' 1
    BLOCK_VERSION_GROESTL        = (2  << 11), //'001000000000000' 2
    BLOCK_VERSION_X17		     = (3  << 11), //'001100000000000' 3
    BLOCK_VERSION_BLAKE 	     = (4  << 11), //'010000000000000' 4
    BLOCK_VERSION_LYRA2RE	     = (10 << 11), //'101000000000000' 10
};

static inline int GetAlgoByName(std::string strAlgo){
    if (strAlgo == "scrypt")
        return ALGO_SCRYPT;
    if (strAlgo == "groestl" || strAlgo == "myr-gr" || strAlgo == "myriad-groestl")
        return ALGO_GROESTL;
    if (strAlgo == "x17")
	    return ALGO_X17;
    if (strAlgo == "lyra" || strAlgo == "lyra2re" || strAlgo == "lyra2v2" || strAlgo == "lyra2" || strAlgo == "lyra2rev2")
	    return ALGO_LYRA2RE;
    if (strAlgo == "blake" || strAlgo == "blake2s")
	    return ALGO_BLAKE;
    //printf("GetAlgoByName(): Can't Parse Algo, %s\n", strAlgo.c_str());
    return ALGO_SCRYPT;
}

static inline std::string GetAlgoName(int algo)
{
    switch (algo)
    {
        case ALGO_SCRYPT:
            return std::string("scrypt");
        case ALGO_GROESTL:
            return std::string("groestl");
        case ALGO_LYRA2RE:
            return std::string("lyra2re");
        case ALGO_BLAKE:
            return std::string("blake");
        case ALGO_X17:
            return std::string("x17");
    }
    return std::string("unknown");
}

/**
 * Current default algo to use from multi algo
 */
extern int ALGO;

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;

    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;
    uint256 GetPoWHash(int algo) const;

    int GetAlgo() const
    {
        switch (nVersion & BLOCK_VERSION_ALGO)
        {
            case BLOCK_VERSION_SCRYPT:
                return ALGO_SCRYPT;
            case BLOCK_VERSION_GROESTL:
                return ALGO_GROESTL;
            case BLOCK_VERSION_LYRA2RE:
                return ALGO_LYRA2RE;
            case BLOCK_VERSION_X17:
                return ALGO_X17;
            case BLOCK_VERSION_BLAKE:
                return ALGO_BLAKE;
        }
        //printf("CBlock::GetAlgo(): Can't Parse Algo, %d, %d, %d\n", nVersion & BLOCK_VERSION_ALGO, nVersion, this->nVersion);
        return ALGO_SCRYPT;
    }

    std::string GetAlgoName() const
    {
        return ::GetAlgoName(GetAlgo());
    }


    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransactionRef> vtx;

    // memory only
    mutable bool fChecked;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *(static_cast<CBlockHeader*>(this)) = header;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITEAS(CBlockHeader, *this);
        READWRITE(vtx);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        fChecked = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        return block;
    }

    std::string ToString() const;
};


/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    explicit CBlockLocator(const std::vector<uint256>& vHaveIn) : vHave(vHaveIn) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        int nVersion = s.GetVersion();
        if (!(s.GetType() & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

#endif // VERGE_PRIMITIVES_BLOCK_H
