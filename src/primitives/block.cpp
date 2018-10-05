// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2018-2018 The VERGE Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <crypto/common.h>

#include <crypto/pow/scrypt.h>
#include <crypto/pow/hashgroestl.h>
#include <crypto/pow/hashblake.h>
#include <crypto/pow/hashx17.h>
#include <crypto/pow/Lyra2RE.h>

int ALGO = ALGO_SCRYPT;

uint256 CBlockHeader::GetHash() const
{
    // return SerializeHash(*this);
    return GetPoWHash(ALGO_SCRYPT);
}

uint256 CBlockHeader::GetPoWHash(int algo) const
{
    uint256 thash;
    switch (algo)
    {
        case ALGO_GROESTL:
            return HashGroestl(BEGIN(nVersion), END(nNonce));
        case ALGO_BLAKE:
            return HashBlake(BEGIN(nVersion), END(nNonce));
        case ALGO_X17:
            return HashX17(BEGIN(nVersion), END(nNonce));
        //case ALGO_LYRA2RE:
        //    return lyra2re2_hash(BEGIN(nVersion), BEGIN(thash));
        case ALGO_SCRYPT:
        default:
            scrypt_1024_1_1_256(BEGIN(nVersion), BEGIN(thash));

    }
    return thash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, algo=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
	GetAlgoName(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
