//
// Created by Xule Zhou on 7/17/18.
//

#include "hap_crypto.h"
#include "crypto/ctr_drbg.h"
#include "crypto/bignum.h"
#include "crypto/sha512.h"
#include "crypto/srp.h"
#include "crypto/chachapoly.h"
#include "HomeKitAccessory.h"

#include <cstring>
#include <random>

static const uint8_t _modulus[] PROGMEM = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc9, 0x0f, 0xda, 0xa2,
        0x21, 0x68, 0xc2, 0x34, 0xc4, 0xc6, 0x62, 0x8b, 0x80, 0xdc, 0x1c, 0xd1,
        0x29, 0x02, 0x4e, 0x08, 0x8a, 0x67, 0xcc, 0x74, 0x02, 0x0b, 0xbe, 0xa6,
        0x3b, 0x13, 0x9b, 0x22, 0x51, 0x4a, 0x08, 0x79, 0x8e, 0x34, 0x04, 0xdd,
        0xef, 0x95, 0x19, 0xb3, 0xcd, 0x3a, 0x43, 0x1b, 0x30, 0x2b, 0x0a, 0x6d,
        0xf2, 0x5f, 0x14, 0x37, 0x4f, 0xe1, 0x35, 0x6d, 0x6d, 0x51, 0xc2, 0x45,
        0xe4, 0x85, 0xb5, 0x76, 0x62, 0x5e, 0x7e, 0xc6, 0xf4, 0x4c, 0x42, 0xe9,
        0xa6, 0x37, 0xed, 0x6b, 0x0b, 0xff, 0x5c, 0xb6, 0xf4, 0x06, 0xb7, 0xed,
        0xee, 0x38, 0x6b, 0xfb, 0x5a, 0x89, 0x9f, 0xa5, 0xae, 0x9f, 0x24, 0x11,
        0x7c, 0x4b, 0x1f, 0xe6, 0x49, 0x28, 0x66, 0x51, 0xec, 0xe4, 0x5b, 0x3d,
        0xc2, 0x00, 0x7c, 0xb8, 0xa1, 0x63, 0xbf, 0x05, 0x98, 0xda, 0x48, 0x36,
        0x1c, 0x55, 0xd3, 0x9a, 0x69, 0x16, 0x3f, 0xa8, 0xfd, 0x24, 0xcf, 0x5f,
        0x83, 0x65, 0x5d, 0x23, 0xdc, 0xa3, 0xad, 0x96, 0x1c, 0x62, 0xf3, 0x56,
        0x20, 0x85, 0x52, 0xbb, 0x9e, 0xd5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6d,
        0x67, 0x0c, 0x35, 0x4e, 0x4a, 0xbc, 0x98, 0x04, 0xf1, 0x74, 0x6c, 0x08,
        0xca, 0x18, 0x21, 0x7c, 0x32, 0x90, 0x5e, 0x46, 0x2e, 0x36, 0xce, 0x3b,
        0xe3, 0x9e, 0x77, 0x2c, 0x18, 0x0e, 0x86, 0x03, 0x9b, 0x27, 0x83, 0xa2,
        0xec, 0x07, 0xa2, 0x8f, 0xb5, 0xc5, 0x5d, 0xf0, 0x6f, 0x4c, 0x52, 0xc9,
        0xde, 0x2b, 0xcb, 0xf6, 0x95, 0x58, 0x17, 0x18, 0x39, 0x95, 0x49, 0x7c,
        0xea, 0x95, 0x6a, 0xe5, 0x15, 0xd2, 0x26, 0x18, 0x98, 0xfa, 0x05, 0x10,
        0x15, 0x72, 0x8e, 0x5a, 0x8a, 0xaa, 0xc4, 0x2d, 0xad, 0x33, 0x17, 0x0d,
        0x04, 0x50, 0x7a, 0x33, 0xa8, 0x55, 0x21, 0xab, 0xdf, 0x1c, 0xba, 0x64,
        0xec, 0xfb, 0x85, 0x04, 0x58, 0xdb, 0xef, 0x0a, 0x8a, 0xea, 0x71, 0x57,
        0x5d, 0x06, 0x0c, 0x7d, 0xb3, 0x97, 0x0f, 0x85, 0xa6, 0xe1, 0xe4, 0xc7,
        0xab, 0xf5, 0xae, 0x8c, 0xdb, 0x09, 0x33, 0xd7, 0x1e, 0x8c, 0x94, 0xe0,
        0x4a, 0x25, 0x61, 0x9d, 0xce, 0xe3, 0xd2, 0x26, 0x1a, 0xd2, 0xee, 0x6b,
        0xf1, 0x2f, 0xfa, 0x06, 0xd9, 0x8a, 0x08, 0x64, 0xd8, 0x76, 0x02, 0x73,
        0x3e, 0xc8, 0x6a, 0x64, 0x52, 0x1f, 0x2b, 0x18, 0x17, 0x7b, 0x20, 0x0c,
        0xbb, 0xe1, 0x17, 0x57, 0x7a, 0x61, 0x5d, 0x6c, 0x77, 0x09, 0x88, 0xc0,
        0xba, 0xd9, 0x46, 0xe2, 0x08, 0xe2, 0x4f, 0xa0, 0x74, 0xe5, 0xab, 0x31,
        0x43, 0xdb, 0x5b, 0xfc, 0xe0, 0xfd, 0x10, 0x8e, 0x4b, 0x82, 0xd1, 0x20,
        0xa9, 0x3a, 0xd2, 0xca, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static const uint8_t _generator[] PROGMEM = {
        0x05
};

static mbedtls_mpi * _mpiNew(){
    auto mpi = new mbedtls_mpi;
    mbedtls_mpi_init(mpi);
    return mpi;
}

static mbedtls_mpi * _mpiNew(const void * bin, size_t len){
    auto mpi = _mpiNew();
    mbedtls_mpi_read_binary(mpi, static_cast<const unsigned char *>(bin), len);
    return mpi;
}

static void _mpiFree(mbedtls_mpi * &mpi){
    mbedtls_mpi_free(mpi);
    delete mpi;
    mpi = nullptr;
}

static NGConstant * _hap_read_ng(){
    auto ng = new NGConstant;

    auto N = new uint8_t[HAPCRYPTO_SRP_MODULUS_SIZE]();
    memcpy_P(N, _modulus, HAPCRYPTO_SRP_MODULUS_SIZE);
    ng->N = _mpiNew();
    mbedtls_mpi_read_binary(ng->N, N, HAPCRYPTO_SRP_MODULUS_SIZE);
    delete[] N;

    auto g = new uint8_t[HAPCRYPTO_SRP_GENERATOR_SIZE]();
    memcpy_P(g, _generator, HAPCRYPTO_SRP_GENERATOR_SIZE);
    ng->g = _mpiNew();
    mbedtls_mpi_read_binary(ng->g, g, HAPCRYPTO_SRP_GENERATOR_SIZE);
    delete[] g;

    return ng;
}

/**
 * alloc & init sha512 ctx
 *
 * @return ctx
 */
static mbedtls_sha512_context * _sha512InitStart(){
    auto ctx = new mbedtls_sha512_context;
    mbedtls_sha512_init(ctx);
    mbedtls_sha512_starts_ret(ctx, 0);
    return ctx;
}

static void _sha512Update(mbedtls_sha512_context * ctx, mbedtls_mpi * mpi){
    auto len = mbedtls_mpi_size(mpi);
    auto buf = new uint8_t[len];
    mbedtls_mpi_write_binary(mpi, buf, len);
    mbedtls_sha512_update_ret(ctx, buf, len);
    delete[] buf;
}

static void _sha512UpdatePad(mbedtls_sha512_context * ctx, NGConstant * ng, mbedtls_mpi * mpi){
    auto totLen = mbedtls_mpi_size(ng->N);
    auto datLen = mbedtls_mpi_size(mpi);

    if(datLen < totLen){
        auto padLen = totLen - datLen;
        auto buf = new uint8_t[padLen]();
        //update padding zeros
        mbedtls_sha512_update_ret(ctx, buf, padLen);
        delete[] buf;
    }

    _sha512Update(ctx, mpi);
}

static uint8_t * _sha512FinalFree(mbedtls_sha512_context * ctx){
    auto buf = new uint8_t[HAPCRYPTO_SHA_SIZE];
    mbedtls_sha512_finish_ret(ctx, buf);
    mbedtls_sha512_free(ctx);
    delete ctx;
    return buf;
}

static mbedtls_mpi * _sha512FinalMpi(mbedtls_sha512_context * ctx){
    auto ret = _sha512FinalFree(ctx);
    auto mpi = _mpiNew();
    mbedtls_mpi_read_binary(mpi, ret, HAPCRYPTO_SHA_SIZE);
    delete[] ret;
    return mpi;
}

/**
 * Generate salt and verifier, then emits HAPCRYPTO_SRP_INIT_FINISH_GEN_SALT
 *
 * @param info
 */
void hap_crypto_srp_init(hap_crypto_setup * info) {
    //generate the 16bytes salt
    auto s = _mpiNew();
    csrp_init_random();
    mbedtls_mpi_fill_random(s, HAPCRYPTO_SALT_SIZE, &mbedtls_ctr_drbg_random, csrp_ctr_drbg_ctx());
    info->salt = new uint8_t[HAPCRYPTO_SALT_SIZE];
    mbedtls_mpi_write_binary(s, const_cast<unsigned char *>(info->salt), HAPCRYPTO_SALT_SIZE);
    _mpiFree(s);

    //x = H(s | H(I | ":" | P))

    auto xctx = _sha512InitStart();
    mbedtls_sha512_update_ret(
            xctx,
            reinterpret_cast<const unsigned char *>(info->username),
            strlen(info->username));
    unsigned char _col = ':';
    mbedtls_sha512_update_ret(xctx, &_col, 1);
    mbedtls_sha512_update_ret(
            xctx,
            reinterpret_cast<const unsigned char *>(info->password),
            strlen(info->password));
    auto xtmp = _sha512FinalFree(xctx);

    xctx = _sha512InitStart();
    mbedtls_sha512_update_ret(xctx, info->salt, HAPCRYPTO_SALT_SIZE);
    mbedtls_sha512_update_ret(xctx, xtmp, HAPCRYPTO_SHA_SIZE);
    delete[] xtmp;
    auto x = _sha512FinalMpi(xctx);

    //v = g^x % N

    auto ng = _hap_read_ng();
    auto v = _mpiNew();
    mbedtls_mpi_exp_mod(v, ng->g, x, ng->N, csrp_speed_RR());
    _mpiFree(x);
    delete_ng(ng);

    info->verifierLen = static_cast<unsigned int>(mbedtls_mpi_size(v));
    info->verifier = new uint8_t[info->verifierLen];
    mbedtls_mpi_write_binary(v, const_cast<unsigned char *>(info->verifier), info->verifierLen);
    _mpiFree(v);

    //Next gen public key
    info->server->emit(HAPEvent::HAPCRYPTO_SRP_INIT_FINISH_GEN_SALT, info);
}

/**
 * Generate public key and emits HAPCRYPTO_SRP_INIT_COMPLETE
 *
 * @param event
 */
void _srpInit_onGenSalt_thenGenPub(HAPEvent * event){
    auto info = event->arg<hap_crypto_setup>();
    auto ng = _hap_read_ng();
    csrp_init_random();

    //Generate private key, b
    auto b = _mpiNew();
    mbedtls_mpi_fill_random(b, 32, &mbedtls_ctr_drbg_random, csrp_ctr_drbg_ctx());

    //Export b
    auto bLen = mbedtls_mpi_size(b);
    auto bBytes = new uint8_t[bLen];
    mbedtls_mpi_write_binary(b, bBytes, bLen);
    info->b = bBytes;
    info->bLen = static_cast<uint16_t>(bLen);

    //k = H(N | PAD(g))
    auto kCtx = _sha512InitStart();
    _sha512Update(kCtx, ng->N);
    _sha512UpdatePad(kCtx, ng, ng->g);
    auto k = _sha512FinalMpi(kCtx);

    auto v = _mpiNew();
    auto tmp1 = _mpiNew();
    auto tmp2 = _mpiNew();

    mbedtls_mpi_read_binary(v, info->verifier, info->verifierLen);

    //B = k*v + g^b % N
    //Calculate two fragments of B

    //tmp1 = k*v
    mbedtls_mpi_mul_mpi(tmp1, k, v);
    //tmp2 = g^b % N
    mbedtls_mpi_exp_mod(tmp2, ng->g, b, ng->N, csrp_speed_RR());

    //Free k, b, v
    _mpiFree(k);
    _mpiFree(v);
    _mpiFree(b);

    //Calculate B, the public key
    auto B = _mpiNew();
    //B = tmp1+tmp2
    mbedtls_mpi_add_mpi(B, tmp1, tmp2);
    _mpiFree(tmp1);
    _mpiFree(tmp2);

    //Last but not least, mod N
    mbedtls_mpi_mod_mpi(B, B, ng->N);

    //No longer need ng, will recreate when M3
    delete_ng(ng);

    //Export B
    auto BLen = mbedtls_mpi_size(B);
    auto BBytes = new uint8_t[BLen];
    mbedtls_mpi_write_binary(B, BBytes, BLen);
    info->B = BBytes;
    info->BLen = static_cast<uint16_t>(BLen);

    //Free B
    _mpiFree(B);

    info->server->emit(HAPEvent::HAPCRYPTO_SRP_INIT_COMPLETE, info);
}

void hap_crypto_srp_proof(hap_crypto_setup * info) {
    auto verifier = new SRPVerifier;
    auto ng = _hap_read_ng();

    info->handle = verifier;
    verifier->authenticated = 0;
    verifier->username = info->username;
    verifier->hash_alg = SRP_SHA512;
    verifier->ng = ng;
    verifier->bytes_B = info->B;

    //Next: gen session key
    info->server->emit(HAPEvent::HAPCRYPTO_SRP_PROOF_VERIFIER_CREATED, info);
}

void _srpProof_onVerifierCreate_thenGenSKey(HAPEvent * event){
    auto info = event->arg<hap_crypto_setup>();
    auto verifier = static_cast<SRPVerifier *>(info->handle);

    auto A = _mpiNew();
    auto B = _mpiNew();
    auto v = _mpiNew();
    mbedtls_mpi_read_binary(A, info->A, info->ALen);
    mbedtls_mpi_read_binary(B, info->B, info->BLen);
    mbedtls_mpi_read_binary(v, info->verifier, info->verifierLen);

    //u = SHA1(PAD(A) | PAD(B))
    auto uCtx = _sha512InitStart();
    _sha512UpdatePad(uCtx, verifier->ng, A);
    _sha512UpdatePad(uCtx, verifier->ng, B);
    auto u = _sha512FinalMpi(uCtx);

    auto tmp1 = _mpiNew();
    auto tmp2 = _mpiNew();

    //Calculate S(premaster secret) = (A * v^u) ^ b % N

    //tmp1 = v ^ u % N
    mbedtls_mpi_exp_mod(tmp1, v, u, verifier->ng->N, csrp_speed_RR());

    _mpiFree(u);
    _mpiFree(v);

    //tmp2 = tmp1 * A % N
    mbedtls_mpi_mul_mpi(tmp2, A, tmp1);
    mbedtls_mpi_mod_mpi(tmp2, tmp2, verifier->ng->N);

    _mpiFree(A);
    _mpiFree(B);
    _mpiFree(tmp1);

    auto b = _mpiNew();
    auto S = _mpiNew();
    mbedtls_mpi_read_binary(b, info->b, info->bLen);

    //S = tmp2 ^ b % N
    mbedtls_mpi_exp_mod(S, tmp2, b, verifier->ng->N, csrp_speed_RR());

    _mpiFree(tmp2);
    _mpiFree(b);

    //Calculate K(session key)

    auto Kctx = _sha512InitStart();
    _sha512Update(Kctx, S);
    _mpiFree(S);
    auto sessionKey = _sha512FinalFree(Kctx);

    //Export session key
    memcpy(verifier->session_key, sessionKey, HAPCRYPTO_SHA_SIZE);
    info->sessionKey = verifier->session_key;

    delete[] sessionKey;

    //Next: M
    info->server->emit(HAPEvent::HAPCRYPTO_SRP_PROOF_SKEY_GENERATED, info);
}

void _srpProof_onSKey_thenM(HAPEvent * event){
    auto info = event->arg<hap_crypto_setup>();
    auto verifier = static_cast<SRPVerifier *>(info->handle);

    //M = H(H(N) xor H(g), H(I), s, A, B, K)

    //HNg = H(N) xor H(g)

    auto tmpCtx = _sha512InitStart();
    _sha512Update(tmpCtx, verifier->ng->N);
    auto HNg = _sha512FinalFree(tmpCtx);

    tmpCtx = _sha512InitStart();
    _sha512Update(tmpCtx, verifier->ng->g);
    auto Hg = _sha512FinalFree(tmpCtx);

    for(uint8_t i = 0; i < HAPCRYPTO_SHA_SIZE; ++i){
        HNg[i] ^= Hg[i];
    }
    delete[] Hg;

    //H(I)
    tmpCtx = _sha512InitStart();
    mbedtls_sha512_update_ret(tmpCtx, reinterpret_cast<const unsigned char *>(info->username), strlen(info->username));
    auto HI = _sha512FinalFree(tmpCtx);

    //Hashing M

    auto MCtx = _sha512InitStart();

    mbedtls_sha512_update_ret(MCtx, HNg, HAPCRYPTO_SHA_SIZE);
    delete[] HNg;

    mbedtls_sha512_update_ret(MCtx, HI, HAPCRYPTO_SHA_SIZE);
    delete[] HI;

    auto s = _mpiNew(info->salt, HAPCRYPTO_SALT_SIZE);
    _sha512Update(MCtx, s);
    _mpiFree(s);

    auto A = _mpiNew(info->A, info->ALen);
    _sha512Update(MCtx, A);
    _mpiFree(A);

    auto B = _mpiNew(info->B, info->BLen);
    _sha512Update(MCtx, B);
    _mpiFree(B);

    auto K = _mpiNew(info->sessionKey, HAPCRYPTO_SHA_SIZE);
    _sha512Update(MCtx, K);
    _mpiFree(K);

    auto MBytes = _sha512FinalFree(MCtx);

    memcpy(verifier->M, MBytes, HAPCRYPTO_SHA_SIZE);
    delete[] MBytes;

    //Next, generate H(A|M|K)
    info->server->emit(HAPEvent::HAPCRYPTO_SRP_PROOF_SSIDE_GENERATED, info);
}

void _srpProof_onM_thenAMK(HAPEvent * event){
    auto info = event->arg<hap_crypto_setup>();
    auto verifier = static_cast<SRPVerifier *>(info->handle);

    auto AMKCtx = _sha512InitStart();
    mbedtls_sha512_update_ret(AMKCtx, info->A, info->ALen);
    mbedtls_sha512_update_ret(AMKCtx, verifier->M, HAPCRYPTO_SHA_SIZE);
    mbedtls_sha512_update_ret(AMKCtx, info->sessionKey, HAPCRYPTO_SHA_SIZE);
    info->serverProof = _sha512FinalFree(AMKCtx);

    //Complete M4
    info->server->emit(HAPEvent::HAPCRYPTO_SRP_PROOF_COMPLETE, info);
}

void _chachaPoly_decrypt(HAPEvent * event){
    auto info = event->arg<hap_crypto_info>();

    unsigned char nonce[12];
    memset(nonce, 0, sizeof(nonce));
    memcpy(nonce + 16 - info->nonceLen, info->nonce, info->nonceLen);

    delete[] info->rawData;
    info->rawData = new uint8_t[info->dataLen];

    auto ctx = new mbedtls_chachapoly_context;
    mbedtls_chachapoly_init(ctx);
    mbedtls_chachapoly_setkey(ctx, info->key);
    auto ret = mbedtls_chachapoly_auth_decrypt(
            ctx, info->dataLen, nonce, info->aad, info->aadLen,
            info->authTag, info->encryptedData, info->rawData
    );
    mbedtls_chachapoly_free(ctx);

    delete ctx;

    if(ret == 0){
        delete[] info->encryptedData;//Free encrypted data after decrypted
        info->encryptedData = nullptr;
    }
}

void hap_crypto_init(HAPServer * server) {
    //M1
    server->on(HAPEvent::HAPCRYPTO_SRP_INIT_FINISH_GEN_SALT, _srpInit_onGenSalt_thenGenPub);
    //M2
    server->on(HAPEvent::HAPCRYPTO_SRP_PROOF_VERIFIER_CREATED, _srpProof_onVerifierCreate_thenGenSKey);
    server->on(HAPEvent::HAPCRYPTO_SRP_PROOF_SKEY_GENERATED, _srpProof_onSKey_thenM);
    server->on(HAPEvent::HAPCRYPTO_SRP_PROOF_SSIDE_GENERATED, _srpProof_onM_thenAMK);

    //Chachapoly decrypt
    server->on(HAPEvent::HAPCRYPTO_NEED_DECRYPT, _chachaPoly_decrypt);
}

void hap_crypto_srp_free(hap_crypto_setup * info) {
    auto verifier = static_cast<SRPVerifier *>(info->handle);
    delete verifier;
    info->handle = nullptr;
}

bool hap_crypto_verify_client_proof(hap_crypto_setup * info) {
    auto verifier = static_cast<SRPVerifier *>(info->handle);
    return memcmp(verifier->M, info->clientProof, HAPCRYPTO_SHA_SIZE) == 0;
}

void hap_crypto_data_decrypt(hap_crypto_info * info) {
    info->server->emit(HAPEvent::HAPCRYPTO_NEED_DECRYPT, info);
}

bool hap_crypto_data_decrypt_did_succeed(hap_crypto_info * info) {
    return info->encryptedData == nullptr;
}

hap_crypto_setup::hap_crypto_setup(HAPServer *server, const char * user, const char * pass):
        server(server), username(user), password(pass) { }

hap_crypto_setup::~hap_crypto_setup() {
    hap_crypto_srp_free(this);
    delete[] salt;
    delete[] verifier;
    delete[] b;
    delete[] B;
    delete[] A;
    //Not deleting session key since its referenced from verifier
    delete[] clientProof;
    //Not deleting server proof since its also ref from ver
}

hap_crypto_info::hap_crypto_info(HAPServer * server, HAPUserHelper * session):
    server(server), session(session){ }

hap_crypto_info::~hap_crypto_info() {
    free();
}

void hap_crypto_info::free() {
    dataLen = 0;
    delete[] encryptedData;
    delete[] rawData;
    //Not deleting authTag, since in every scenario auth tag is appended after encryptedData
//    delete[] authTag;
    encryptedData = nullptr;
    rawData = nullptr;
    authTag = nullptr;
}