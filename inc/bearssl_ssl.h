/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BR_BEARSSL_SSL_H__
#define BR_BEARSSL_SSL_H__

#include <stddef.h>
#include <stdint.h>

#include "bearssl_block.h"
#include "bearssl_hash.h"
#include "bearssl_hmac.h"
#include "bearssl_prf.h"
#include "bearssl_rand.h"
#include "bearssl_x509.h"

/** \file bearssl_ssl.h
 *
 * # SSL
 *
 * For an overview of the SSL/TLS API, see [the BearSSL Web
 * site](https://www.bearssl.org/api1.html).
 *
 * The `BR_TLS_*` constants correspond to the standard cipher suites and
 * their values in the [IANA
 * registry](http://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-4).
 *
 * The `BR_ALERT_*` constants are for standard TLS alert messages. When
 * a fatal alert message is sent of received, then the SSL engine context
 * status is set to the sum of that alert value (an integer in the 0..255
 * range) and a fixed offset (`BR_ERR_SEND_FATAL_ALERT` for a sent alert,
 * `BR_ERR_RECV_FATAL_ALERT` for a received alert).
 */

/** \brief Optimal input buffer size. */
#define BR_SSL_BUFSIZE_INPUT    (16384 + 325)

/** \brief Optimal output buffer size. */
#define BR_SSL_BUFSIZE_OUTPUT   (16384 + 85)

/** \brief Optimal buffer size for monodirectional engine
    (shared input/output buffer). */
#define BR_SSL_BUFSIZE_MONO     BR_SSL_BUFSIZE_INPUT

/** \brief Optimal buffer size for bidirectional engine
    (single buffer split into two separate input/output buffers). */
#define BR_SSL_BUFSIZE_BIDI     (BR_SSL_BUFSIZE_INPUT + BR_SSL_BUFSIZE_OUTPUT)

/*
 * Constants for known SSL/TLS protocol versions (SSL 3.0, TLS 1.0, TLS 1.1
 * and TLS 1.2). Note that though there is a constant for SSL 3.0, that
 * protocol version is not actually supported.
 */

/** \brief Protocol version: SSL 3.0 (unsupported). */
#define BR_SSL30   0x0300
/** \brief Protocol version: TLS 1.0. */
#define BR_TLS10   0x0301
/** \brief Protocol version: TLS 1.1. */
#define BR_TLS11   0x0302
/** \brief Protocol version: TLS 1.2. */
#define BR_TLS12   0x0303

/*
 * Error constants. They are used to report the reason why a context has
 * been marked as failed.
 *
 * Implementation note: SSL-level error codes should be in the 1..31
 * range. The 32..63 range is for certificate decoding and validation
 * errors. Received fatal alerts imply an error code in the 256..511 range.
 */

/** \brief SSL status: no error so far (0). */
#define BR_ERR_OK                      0

/** \brief SSL status: caller-provided parameter is incorrect. */
#define BR_ERR_BAD_PARAM               1

/** \brief SSL status: operation requested by the caller cannot be applied
    with the current context state (e.g. reading data while outgoing data
    is waiting to be sent). */
#define BR_ERR_BAD_STATE               2

/** \brief SSL status: incoming protocol or record version is unsupported. */
#define BR_ERR_UNSUPPORTED_VERSION     3

/** \brief SSL status: incoming record version does not match the expected
    version. */
#define BR_ERR_BAD_VERSION             4

/** \brief SSL status: incoming record length is invalid. */
#define BR_ERR_BAD_LENGTH              5

/** \brief SSL status: incoming record is too large to be processed, or
    buffer is too small for the handshake message to send. */
#define BR_ERR_TOO_LARGE               6

/** \brief SSL status: decryption found an invalid padding, or the record
    MAC is not correct. */
#define BR_ERR_BAD_MAC                 7

/** \brief SSL status: no initial entropy was provided, and none can be
    obtained from the OS. */
#define BR_ERR_NO_RANDOM               8

/** \brief SSL status: incoming record type is unknown. */
#define BR_ERR_UNKNOWN_TYPE            9

/** \brief SSL status: incoming record or message has wrong type with
    regards to the current engine state. */
#define BR_ERR_UNEXPECTED             10

/** \brief SSL status: ChangeCipherSpec message from the peer has invalid
    contents. */
#define BR_ERR_BAD_CCS                12

/** \brief SSL status: alert message from the peer has invalid contents
    (odd length). */
#define BR_ERR_BAD_ALERT              13

/** \brief SSL status: incoming handshake message decoding failed. */
#define BR_ERR_BAD_HANDSHAKE          14

/** \brief SSL status: ServerHello contains a session ID which is larger
    than 32 bytes. */
#define BR_ERR_OVERSIZED_ID           15

/** \brief SSL status: server wants to use a cipher suite that we did
    not claim to support. This is also reported if we tried to advertise
    a cipher suite that we do not support. */
#define BR_ERR_BAD_CIPHER_SUITE       16

/** \brief SSL status: server wants to use a compression that we did not
    claim to support. */
#define BR_ERR_BAD_COMPRESSION        17

/** \brief SSL status: server's max fragment length does not match
    client's. */
#define BR_ERR_BAD_FRAGLEN            18

/** \brief SSL status: secure renegotiation failed. */
#define BR_ERR_BAD_SECRENEG           19

/** \brief SSL status: server sent an extension type that we did not
    announce, or used the same extension type several times in a single
    ServerHello. */
#define BR_ERR_EXTRA_EXTENSION        20

/** \brief SSL status: invalid Server Name Indication contents (when
    used by the server, this extension shall be empty). */
#define BR_ERR_BAD_SNI                21

/** \brief SSL status: invalid ServerHelloDone from the server (length
    is not 0). */
#define BR_ERR_BAD_HELLO_DONE         22

/** \brief SSL status: internal limit exceeded (e.g. server's public key
    is too large). */
#define BR_ERR_LIMIT_EXCEEDED         23

/** \brief SSL status: Finished message from peer does not match the
    expected value. */
#define BR_ERR_BAD_FINISHED           24

/** \brief SSL status: session resumption attempt with distinct version
    or cipher suite. */
#define BR_ERR_RESUME_MISMATCH        25

/** \brief SSL status: unsupported or invalid algorithm (ECDHE curve,
    signature algorithm, hash function). */
#define BR_ERR_INVALID_ALGORITHM      26

/** \brief SSL status: invalid signature on ServerKeyExchange message. */
#define BR_ERR_BAD_SIGNATURE          27

/** \brief SSL status: I/O error or premature close on underlying
    transport stream. This error code is set only by the simplified
    I/O API ("br_sslio_*"). */
#define BR_ERR_IO                     31

/** \brief SSL status: base value for a received fatal alert.

    When a fatal alert is received from the peer, the alert value
    is added to this constant. */
#define BR_ERR_RECV_FATAL_ALERT      256

/** \brief SSL status: base value for a sent fatal alert.

    When a fatal alert is sent to the peer, the alert value is added
    to this constant. */
#define BR_ERR_SEND_FATAL_ALERT      512

/* ===================================================================== */

/**
 * \brief Decryption engine for SSL.
 *
 * When processing incoming records, the SSL engine will use a decryption
 * engine that uses a specific context structure, and has a set of
 * methods (a vtable) that follows this template.
 *
 * The decryption engine is responsible for applying decryption, verifying
 * MAC, and keeping track of the record sequence number.
 */
typedef struct br_sslrec_in_class_ br_sslrec_in_class;
struct br_sslrec_in_class_ {
	/**
	 * \brief Context size (in bytes).
	 */
	size_t context_size;

	/**
	 * \brief Test validity of the incoming record length.
	 *
	 * This function returns 1 if the announced length for an
	 * incoming record is valid, 0 otherwise,
	 *
	 * \param ctx          decryption engine context.
	 * \param record_len   incoming record length.
	 * \return  1 of a valid length, 0 otherwise.
	 */
	int (*check_length)(const br_sslrec_in_class *const *ctx,
		size_t record_len);

	/**
	 * \brief Decrypt the incoming record.
	 *
	 * This function may assume that the record length is valid
	 * (it has been previously tested with `check_length()`).
	 * Decryption is done in place; `*len` is updated with the
	 * cleartext length, and the address of the first plaintext
	 * byte is returned. If the record is correct but empty, then
	 * `*len` is set to 0 and a non-`NULL` pointer is returned.
	 *
	 * On decryption/MAC error, `NULL` is returned.
	 *
	 * \param ctx           decryption engine context.
	 * \param record_type   record type (23 for application data, etc).
	 * \param version       record version.
	 * \param payload       address of encrypted payload.
	 * \param len           pointer to payload length (updated).
	 * \return  pointer to plaintext, or `NULL` on error.
	 */
	unsigned char *(*decrypt)(const br_sslrec_in_class **ctx,
		int record_type, unsigned version,
		void *payload, size_t *len);
};

/**
 * \brief Encryption engine for SSL.
 *
 * When building outgoing records, the SSL engine will use an encryption
 * engine that uses a specific context structure, and has a set of
 * methods (a vtable) that follows this template.
 *
 * The encryption engine is responsible for applying encryption and MAC,
 * and keeping track of the record sequence number.
 */
typedef struct br_sslrec_out_class_ br_sslrec_out_class;
struct br_sslrec_out_class_ {
	/**
	 * \brief Context size (in bytes).
	 */
	size_t context_size;

	/**
	 * \brief Compute maximum plaintext sizes and offsets.
	 *
	 * When this function is called, the `*start` and `*end`
	 * values contain offsets designating the free area in the
	 * outgoing buffer for plaintext data; that free area is
	 * preceded by a 5-byte space which will receive the record
	 * header.
	 *
	 * The `max_plaintext()` function is responsible for adjusting
	 * both `*start` and `*end` to make room for any record-specific
	 * header, MAC, padding, and possible split.
	 *
	 * \param ctx     encryption engine context.
	 * \param start   pointer to start of plaintext offset (updated).
	 * \param end     pointer to start of plaintext offset (updated).
	 */
	void (*max_plaintext)(const br_sslrec_out_class *const *ctx,
		size_t *start, size_t *end);

	/**
	 * \brief Perform record encryption.
	 *
	 * This function encrypts the record. The plaintext address and
	 * length are provided. Returned value is the start of the
	 * encrypted record (or sequence of records, if a split was
	 * performed), _including_ the 5-byte header, and `*len` is
	 * adjusted to the total size of the record(s), there again
	 * including the header(s).
	 *
	 * \param ctx           decryption engine context.
	 * \param record_type   record type (23 for application data, etc).
	 * \param version       record version.
	 * \param plaintext     address of plaintext.
	 * \param len           pointer to plaintext length (updated).
	 * \return  pointer to start of built record.
	 */
	unsigned char *(*encrypt)(const br_sslrec_out_class **ctx,
		int record_type, unsigned version,
		void *plaintext, size_t *len);
};

/**
 * \brief Context for a no-encryption engine.
 *
 * The no-encryption engine processes outgoing records during the initial
 * handshake, before encryption is applied.
 */
typedef struct {
	/** \brief No-encryption engine vtable. */
	const br_sslrec_out_class *vtable;
} br_sslrec_out_clear_context;

/** \brief Static, constant vtable for the no-encryption engine. */
extern const br_sslrec_out_class br_sslrec_out_clear_vtable;

/* ===================================================================== */

/**
 * \brief Record decryption engine class, for CBC mode.
 *
 * This class type extends the decryption engine class with an
 * initialisation method that receives the parameters needed
 * for CBC processing: block cipher implementation, block cipher key,
 * HMAC parameters (hash function, key, MAC length), and IV. If the
 * IV is `NULL`, then a per-record IV will be used (TLS 1.1+).
 */
typedef struct br_sslrec_in_cbc_class_ br_sslrec_in_cbc_class;
struct br_sslrec_in_cbc_class_ {
	/**
	 * \brief Superclass, as first vtable field.
	 */
	br_sslrec_in_class inner;

	/**
	 * \brief Engine initialisation method.
	 *
	 * This method sets the vtable field in the context.
	 *
	 * \param ctx           context to initialise.
	 * \param bc_impl       block cipher implementation (CBC decryption).
	 * \param bc_key        block cipher key.
	 * \param bc_key_len    block cipher key length (in bytes).
	 * \param dig_impl      hash function for HMAC.
	 * \param mac_key       HMAC key.
	 * \param mac_key_len   HMAC key length (in bytes).
	 * \param mac_out_len   HMAC output length (in bytes).
	 * \param iv            initial IV (or `NULL`).
	 */
	void (*init)(const br_sslrec_in_cbc_class **ctx,
		const br_block_cbcdec_class *bc_impl,
		const void *bc_key, size_t bc_key_len,
		const br_hash_class *dig_impl,
		const void *mac_key, size_t mac_key_len, size_t mac_out_len,
		const void *iv);
};

/**
 * \brief Record encryption engine class, for CBC mode.
 *
 * This class type extends the encryption engine class with an
 * initialisation method that receives the parameters needed
 * for CBC processing: block cipher implementation, block cipher key,
 * HMAC parameters (hash function, key, MAC length), and IV. If the
 * IV is `NULL`, then a per-record IV will be used (TLS 1.1+).
 */
typedef struct br_sslrec_out_cbc_class_ br_sslrec_out_cbc_class;
struct br_sslrec_out_cbc_class_ {
	/**
	 * \brief Superclass, as first vtable field.
	 */
	br_sslrec_out_class inner;

	/**
	 * \brief Engine initialisation method.
	 *
	 * This method sets the vtable field in the context.
	 *
	 * \param ctx           context to initialise.
	 * \param bc_impl       block cipher implementation (CBC encryption).
	 * \param bc_key        block cipher key.
	 * \param bc_key_len    block cipher key length (in bytes).
	 * \param dig_impl      hash function for HMAC.
	 * \param mac_key       HMAC key.
	 * \param mac_key_len   HMAC key length (in bytes).
	 * \param mac_out_len   HMAC output length (in bytes).
	 * \param iv            initial IV (or `NULL`).
	 */
	void (*init)(const br_sslrec_out_cbc_class **ctx,
		const br_block_cbcenc_class *bc_impl,
		const void *bc_key, size_t bc_key_len,
		const br_hash_class *dig_impl,
		const void *mac_key, size_t mac_key_len, size_t mac_out_len,
		const void *iv);
};

/**
 * \brief Context structure for decrypting incoming records with
 * CBC + HMAC.
 *
 * The first field points to the vtable. The other fields are opaque
 * and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_sslrec_in_cbc_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint64_t seq;
	union {
		const br_block_cbcdec_class *vtable;
		br_aes_gen_cbcdec_keys aes;
		br_des_gen_cbcdec_keys des;
	} bc;
	br_hmac_key_context mac;
	size_t mac_len;
	unsigned char iv[16];
	int explicit_IV;
#endif
} br_sslrec_in_cbc_context;

/**
 * \brief Static, constant vtable for record decryption with CBC.
 */
extern const br_sslrec_in_cbc_class br_sslrec_in_cbc_vtable;

/**
 * \brief Context structure for encrypting outgoing records with
 * CBC + HMAC.
 *
 * The first field points to the vtable. The other fields are opaque
 * and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_sslrec_out_cbc_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint64_t seq;
	union {
		const br_block_cbcenc_class *vtable;
		br_aes_gen_cbcenc_keys aes;
		br_des_gen_cbcenc_keys des;
	} bc;
	br_hmac_key_context mac;
	size_t mac_len;
	unsigned char iv[16];
	int explicit_IV;
#endif
} br_sslrec_out_cbc_context;

/**
 * \brief Static, constant vtable for record encryption with CBC.
 */
extern const br_sslrec_out_cbc_class br_sslrec_out_cbc_vtable;

/* ===================================================================== */

/**
 * \brief Record decryption engine class, for GCM mode.
 *
 * This class type extends the decryption engine class with an
 * initialisation method that receives the parameters needed
 * for GCM processing: block cipher implementation, block cipher key,
 * GHASH implementtion, and 4-byte IV.
 */
typedef struct br_sslrec_in_gcm_class_ br_sslrec_in_gcm_class;
struct br_sslrec_in_gcm_class_ {
	/**
	 * \brief Superclass, as first vtable field.
	 */
	br_sslrec_in_class inner;

	/**
	 * \brief Engine initialisation method.
	 *
	 * This method sets the vtable field in the context.
	 *
	 * \param ctx           context to initialise.
	 * \param bc_impl       block cipher implementation (CTR).
	 * \param key           block cipher key.
	 * \param key_len       block cipher key length (in bytes).
	 * \param gh_impl       GHASH implementation.
	 * \param iv            static IV (4 bytes).
	 */
	void (*init)(const br_sslrec_in_gcm_class **ctx,
		const br_block_ctr_class *bc_impl,
		const void *key, size_t key_len,
		br_ghash gh_impl,
		const void *iv);
};

/**
 * \brief Record decryption engine class, for GCM mode.
 *
 * This class type extends the decryption engine class with an
 * initialisation method that receives the parameters needed
 * for GCM processing: block cipher implementation, block cipher key,
 * GHASH implementtion, and 4-byte IV.
 */
typedef struct br_sslrec_out_gcm_class_ br_sslrec_out_gcm_class;
struct br_sslrec_out_gcm_class_ {
	/**
	 * \brief Superclass, as first vtable field.
	 */
	br_sslrec_out_class inner;

	/**
	 * \brief Engine initialisation method.
	 *
	 * This method sets the vtable field in the context.
	 *
	 * \param ctx           context to initialise.
	 * \param bc_impl       block cipher implementation (CTR).
	 * \param key           block cipher key.
	 * \param key_len       block cipher key length (in bytes).
	 * \param gh_impl       GHASH implementation.
	 * \param iv            static IV (4 bytes).
	 */
	void (*init)(const br_sslrec_out_gcm_class **ctx,
		const br_block_ctr_class *bc_impl,
		const void *key, size_t key_len,
		br_ghash gh_impl,
		const void *iv);
};

/**
 * \brief Context structure for processing records with GCM.
 *
 * The same context structure is used for encrypting and decrypting.
 *
 * The first field points to the vtable. The other fields are opaque
 * and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	union {
		const void *gen;
		const br_sslrec_in_gcm_class *in;
		const br_sslrec_out_gcm_class *out;
	} vtable;
#ifndef BR_DOXYGEN_IGNORE
	uint64_t seq;
	union {
		const br_block_ctr_class *vtable;
		br_aes_gen_ctr_keys aes;
	} bc;
	br_ghash gh;
	unsigned char iv[4];
	unsigned char h[16];
#endif
} br_sslrec_gcm_context;

/**
 * \brief Static, constant vtable for record decryption with GCM.
 */
extern const br_sslrec_in_gcm_class br_sslrec_in_gcm_vtable;

/**
 * \brief Static, constant vtable for record encryption with GCM.
 */
extern const br_sslrec_out_gcm_class br_sslrec_out_gcm_vtable;

/* ===================================================================== */

/**
 * \brief Type for session parameters, to be saved for session resumption.
 */
typedef struct {
	/** \brief Session ID buffer. */
	unsigned char session_id[32];
	/** \brief Session ID length (in bytes, at most 32). */
	unsigned char session_id_len;
	/** \brief Protocol version. */
	uint16_t version;
	/** \brief Cipher suite. */
	uint16_t cipher_suite;
	/** \brief Master secret. */
	unsigned char master_secret[48];
} br_ssl_session_parameters;

#ifndef BR_DOXYGEN_IGNORE
/*
 * Maximum numnber of cipher suites supported by a client or server.
 */
#define BR_MAX_CIPHER_SUITES   40
#endif

/**
 * \brief Context structure for SSL engine.
 *
 * This strucuture is common to the client and server; both the client
 * context (`br_ssl_client_context`) and the server context
 * (`br_ssl_server_context`) include a `br_ssl_engine_context` as their
 * first field.
 *
 * The engine context manages records, including alerts, closures, and
 * transitions to new encryption/MAC algorithms. Processing of handshake
 * records is delegated to externally provided code. This structure
 * should not be used directly.
 *
 * Structure contents are opaque and shall not be accessed directly.
 */
typedef struct {
#ifndef BR_DOXYGEN_IGNORE
	/*
	 * The error code. When non-zero, then the state is "failed" and
	 * no I/O may occur until reset.
	 */
	int err;

	/*
	 * Configured I/O buffers. They are either disjoint, or identical.
	 */
	unsigned char *ibuf, *obuf;
	size_t ibuf_len, obuf_len;

	/*
	 * Maximum fragment length applies to outgoing records; incoming
	 * records can be processed as long as they fit in the input
	 * buffer. It is guaranteed that incoming records at least as big
	 * as max_frag_len can be processed.
	 */
	uint16_t max_frag_len;
	unsigned char log_max_frag_len;
	unsigned char peer_log_max_frag_len;

	/*
	 * Buffering management registers.
	 */
	size_t ixa, ixb, ixc;
	size_t oxa, oxb, oxc;
	unsigned char iomode;
	unsigned char incrypt;

	/*
	 * Shutdown flag: when set to non-zero, incoming record bytes
	 * will not be accepted anymore. This is used after a close_notify
	 * has been received: afterwards, the engine no longer claims that
	 * it could receive bytes from the transport medium.
	 */
	unsigned char shutdown_recv;

	/*
	 * 'record_type_in' is set to the incoming record type when the
	 * record header has been received.
	 * 'record_type_out' is used to make the next outgoing record
	 * header when it is ready to go.
	 */
	unsigned char record_type_in, record_type_out;

	/*
	 * When a record is received, its version is extracted:
	 * -- if 'version_in' is 0, then it is set to the received version;
	 * -- otherwise, if the received version is not identical to
	 *    the 'version_in' contents, then a failure is reported.
	 *
	 * This implements the SSL requirement that all records shall
	 * use the negotiated protocol version, once decided (in the
	 * ServerHello). It is up to the handshake handler to adjust this
	 * field when necessary.
	 */
	uint16_t version_in;

	/*
	 * 'version_out' is used when the next outgoing record is ready
	 * to go.
	 */
	uint16_t version_out;

	/*
	 * Record handler contexts.
	 */
	union {
		const br_sslrec_in_class *vtable;
		br_sslrec_in_cbc_context cbc;
		br_sslrec_gcm_context gcm;
	} in;
	union {
		const br_sslrec_out_class *vtable;
		br_sslrec_out_clear_context clear;
		br_sslrec_out_cbc_context cbc;
		br_sslrec_gcm_context gcm;
	} out;

	/*
	 * The "application data" flag. It is set when application data
	 * can be exchanged, cleared otherwise.
	 */
	unsigned char application_data;

	/*
	 * Context RNG.
	 */
	br_hmac_drbg_context rng;
	int rng_init_done;
	int rng_os_rand_done;

	/*
	 * Supported minimum and maximum versions, and cipher suites.
	 */
	uint16_t version_min;
	uint16_t version_max;
	uint16_t suites_buf[BR_MAX_CIPHER_SUITES];
	unsigned char suites_num;

	/*
	 * For clients, the server name to send as a SNI extension. For
	 * servers, the name received in the SNI extension (if any).
	 */
	char server_name[256];

	/*
	 * "Security parameters". These are filled by the handshake
	 * handler, and used when switching encryption state.
	 */
	unsigned char client_random[32];
	unsigned char server_random[32];
	br_ssl_session_parameters session;

	/*
	 * ECDHE elements: curve and point from the peer. The server also
	 * uses that buffer for the point to send to the client.
	 */
	unsigned char ecdhe_curve;
	unsigned char ecdhe_point[133];
	unsigned char ecdhe_point_len;

	/*
	 * Secure renegotiation (RFC 5746): 'reneg' can be:
	 *   0   first handshake (server support is not known)
	 *   1   server does not support secure renegotiation
	 *   2   server supports secure renegotiation
	 *
	 * The saved_finished buffer contains the client and the
	 * server "Finished" values from the last handshake, in
	 * that order (12 bytes each).
	 */
	unsigned char reneg;
	unsigned char saved_finished[24];

	/*
	 * Behavioural flags.
	 */
	uint32_t flags;

	/*
	 * Context variables for the handshake processor.
	 * The 'pad' must be large enough to accommodate an
	 * RSA-encrypted pre-master secret, or a RSA signature on
	 * key exchange parameters; since we want to support up to
	 * RSA-4096, this means at least 512 bytes.
	 * (Other pad usages require its length to be at least 256.)
	 */
	struct {
		uint32_t *dp;
		uint32_t *rp;
		const unsigned char *ip;
	} cpu;
	uint32_t dp_stack[32];
	uint32_t rp_stack[32];
	unsigned char pad[512];
	unsigned char *hbuf_in, *hbuf_out, *saved_hbuf_out;
	size_t hlen_in, hlen_out;
	void (*hsrun)(void *ctx);

	/*
	 * The 'action' value communicates OOB information between the
	 * engine and the handshake processor.
	 *
	 * From the engine:
	 *   0  invocation triggered by I/O
	 *   1  invocation triggered by explicit close
	 *   2  invocation triggered by explicit renegotiation
	 */
	unsigned char action;

	/*
	 * State for alert messages. Value is either 0, or the value of
	 * the alert level byte (level is either 1 for warning, or 2 for
	 * fatal; we convert all other values to 'fatal').
	 */
	unsigned char alert;

	/*
	 * Closure flags. This flag is set when a close_notify has been
	 * received from the peer.
	 */
	unsigned char close_received;

	/*
	 * Multi-hasher for the handshake messages. The handshake handler
	 * is responsible for resetting it when appropriate.
	 */
	br_multihash_context mhash;

	/*
	 * Pointer to the X.509 engine. The engine is supposed to be
	 * already initialized. It is used to validate the peer's
	 * certificate.
	 */
	const br_x509_class **x509ctx;

	/*
	 * Pointers to implementations; left to NULL for unsupported
	 * functions. For the raw hash functions, implementations are
	 * referenced from the multihasher (mhash field).
	 */
	br_tls_prf_impl prf10;
	br_tls_prf_impl prf_sha256;
	br_tls_prf_impl prf_sha384;
	const br_block_cbcenc_class *iaes_cbcenc;
	const br_block_cbcdec_class *iaes_cbcdec;
	const br_block_ctr_class *iaes_ctr;
	const br_block_cbcenc_class *ides_cbcenc;
	const br_block_cbcdec_class *ides_cbcdec;
	br_ghash ighash;
	const br_sslrec_in_cbc_class *icbc_in;
	const br_sslrec_out_cbc_class *icbc_out;
	const br_sslrec_in_gcm_class *igcm_in;
	const br_sslrec_out_gcm_class *igcm_out;
	const br_ec_impl *iec;
#endif
} br_ssl_engine_context;

/**
 * \brief Get currently defined engine behavioural flags.
 *
 * \param cc   SSL engine context.
 * \return  the flags.
 */
static inline uint32_t
br_ssl_engine_get_flags(br_ssl_engine_context *cc)
{
	return cc->flags;
}

/**
 * \brief Set all engine behavioural flags.
 *
 * \param cc      SSL engine context.
 * \param flags   new value for all flags.
 */
static inline void
br_ssl_engine_set_all_flags(br_ssl_engine_context *cc, uint32_t flags)
{
	cc->flags = flags;
}

/**
 * \brief Set some engine behavioural flags.
 *
 * The flags set in the `flags` parameter are set in the context; other
 * flags are untouched.
 *
 * \param cc      SSL engine context.
 * \param flags   additional set flags.
 */
static inline void
br_ssl_engine_add_flags(br_ssl_engine_context *cc, uint32_t flags)
{
	cc->flags |= flags;
}

/**
 * \brief Clear some engine behavioural flags.
 *
 * The flags set in the `flags` parameter are cleared from the context; other
 * flags are untouched.
 *
 * \param cc      SSL engine context.
 * \param flags   flags to remove.
 */
static inline void
br_ssl_engine_remove_flags(br_ssl_engine_context *cc, uint32_t flags)
{
	cc->flags &= ~flags;
}

/**
 * \brief Behavioural flag: enforce server preferences.
 *
 * If this flag is set, then the server will enforce its own cipher suite
 * preference order; otherwise, it follows the client preferences.
 */
#define BR_OPT_ENFORCE_SERVER_PREFERENCES      ((uint32_t)1 << 0)

/*
 * \brief Behavioural flag: disable renegotiation.
 *
 * If this flag is set, then renegotiations are rejected unconditionally:
 * they won't be honoured if asked for programmatically, and requests from
 * the peer are rejected.
 */
#define BR_OPT_NO_RENEGOTIATION                ((uint32_t)1 << 1)

/**
 * \brief Set the minimum and maximum supported protocol versions.
 *
 * The two provided versions MUST be supported by the implementation
 * (i.e. TLS 1.0, 1.1 and 1.2), and `version_max` MUST NOT be lower
 * than `version_min`.
 *
 * \param cc            SSL engine context.
 * \param version_min   minimum supported TLS version.
 * \param version_max   maximum supported TLS version.
 */
static inline void
br_ssl_engine_set_versions(br_ssl_engine_context *cc,
	unsigned version_min, unsigned version_max)
{
	cc->version_min = version_min;
	cc->version_max = version_max;
}

/**
 * \brief Set the list of cipher suites advertised by this context.
 *
 * The provided array is copied into the context. It is the caller
 * responsibility to ensure that all provided suites will be supported
 * by the context. The engine context has enough room to receive _all_
 * suites supported by the implementation. The provided array MUST NOT
 * contain duplicates.
 *
 * If the engine is for a client, the "signaling" pseudo-cipher suite
 * `TLS_FALLBACK_SCSV` can be added at the end of the list, if the
 * calling application is performing a voluntary downgrade (voluntary
 * downgrades are not recommended, but if such a downgrade is done, then
 * adding the fallback pseudo-suite is a good idea).
 *
 * \param cc           SSL engine context.
 * \param suites       cipher suites.
 * \param suites_num   number of cipher suites.
 */
void br_ssl_engine_set_suites(br_ssl_engine_context *cc,
	const uint16_t *suites, size_t suites_num);

/**
 * \brief Set the X.509 engine.
 *
 * The caller shall ensure that the X.509 engine is properly initialised.
 *
 * \param cc        SSL engine context.
 * \param x509ctx   X.509 certificate validation context.
 */
static inline void
br_ssl_engine_set_x509(br_ssl_engine_context *cc, const br_x509_class **x509ctx)
{
	cc->x509ctx = x509ctx;
}

/**
 * \brief Set a hash function implementation (by ID).
 *
 * Hash functions set with this call will be used for SSL/TLS specific
 * usages, not X.509 certificate validation. Only "standard" hash functions
 * may be set (MD5, SHA-1, SHA-224, SHA-256, SHA-384, SHA-512). If `impl`
 * is `NULL`, then the hash function support is removed, not added.
 *
 * \param ctx    SSL engine context.
 * \param id     hash function identifier.
 * \param impl   hash function implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_hash(br_ssl_engine_context *ctx,
	int id, const br_hash_class *impl)
{
	br_multihash_setimpl(&ctx->mhash, id, impl);
}

/**
 * \brief Get a hash function implementation (by ID).
 *
 * This function retrieves a hash function implementation which was
 * set with `br_ssl_engine_set_hash()`.
 *
 * \param ctx   SSL engine context.
 * \param id    hash function identifier.
 * \return  the hash function implementation (or `NULL`).
 */
static inline const br_hash_class *
br_ssl_engine_get_hash(br_ssl_engine_context *ctx, int id)
{
	return br_multihash_getimpl(&ctx->mhash, id);
}

/**
 * \brief Set the PRF implementation (for TLS 1.0 and 1.1).
 *
 * This function sets (or removes, if `impl` is `NULL`) the implemenation
 * for the PRF used in TLS 1.0 and 1.1.
 *
 * \param cc     SSL engine context.
 * \param impl   PRF implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_prf10(br_ssl_engine_context *cc, br_tls_prf_impl impl)
{
	cc->prf10 = impl;
}

/**
 * \brief Set the PRF implementation with SHA-256 (for TLS 1.2).
 *
 * This function sets (or removes, if `impl` is `NULL`) the implemenation
 * for the SHA-256 variant of the PRF used in TLS 1.2.
 *
 * \param cc     SSL engine context.
 * \param impl   PRF implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_prf_sha256(br_ssl_engine_context *cc, br_tls_prf_impl impl)
{
	cc->prf_sha256 = impl;
}

/**
 * \brief Set the PRF implementation with SHA-384 (for TLS 1.2).
 *
 * This function sets (or removes, if `impl` is `NULL`) the implemenation
 * for the SHA-384 variant of the PRF used in TLS 1.2.
 *
 * \param cc     SSL engine context.
 * \param impl   PRF implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_prf_sha384(br_ssl_engine_context *cc, br_tls_prf_impl impl)
{
	cc->prf_sha384 = impl;
}

/**
 * \brief Set the AES/CBC implementations.
 *
 * \param cc         SSL engine context.
 * \param impl_enc   AES/CBC encryption implementation (or `NULL`).
 * \param impl_dec   AES/CBC decryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_aes_cbc(br_ssl_engine_context *cc,
	const br_block_cbcenc_class *impl_enc,
	const br_block_cbcdec_class *impl_dec)
{
	cc->iaes_cbcenc = impl_enc;
	cc->iaes_cbcdec = impl_dec;
}

/**
 * \brief Set the AES/CTR implementation.
 *
 * \param cc     SSL engine context.
 * \param impl   AES/CTR encryption/decryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_aes_ctr(br_ssl_engine_context *cc,
	const br_block_ctr_class *impl)
{
	cc->iaes_ctr = impl;
}

/**
 * \brief Set the DES/CBC implementations.
 *
 * \param cc         SSL engine context.
 * \param impl_enc   DES/CBC encryption implementation (or `NULL`).
 * \param impl_dec   DES/CBC decryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_des_cbc(br_ssl_engine_context *cc,
	const br_block_cbcenc_class *impl_enc,
	const br_block_cbcdec_class *impl_dec)
{
	cc->ides_cbcenc = impl_enc;
	cc->ides_cbcdec = impl_dec;
}

/**
 * \brief Set the GHASH implementation (used in GCM mode).
 *
 * \param cc     SSL engine context.
 * \param impl   GHASH implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_ghash(br_ssl_engine_context *cc, br_ghash impl)
{
	cc->ighash = impl;
}

/**
 * \brief Set the record encryption and decryption engines for CBC + HMAC.
 *
 * \param cc         SSL engine context.
 * \param impl_in    record CBC decryption implementation (or `NULL`).
 * \param impl_out   record CBC encryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_cbc(br_ssl_engine_context *cc,
	const br_sslrec_in_cbc_class *impl_in,
	const br_sslrec_out_cbc_class *impl_out)
{
	cc->icbc_in = impl_in;
	cc->icbc_out = impl_out;
}

/**
 * \brief Set the record encryption and decryption engines for GCM.
 *
 * \param cc         SSL engine context.
 * \param impl_in    record GCM decryption implementation (or `NULL`).
 * \param impl_out   record GCM encryption implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_gcm(br_ssl_engine_context *cc,
	const br_sslrec_in_gcm_class *impl_in,
	const br_sslrec_out_gcm_class *impl_out)
{
	cc->igcm_in = impl_in;
	cc->igcm_out = impl_out;
}

/**
 * \brief Set the EC implementation.
 *
 * The elliptic curve implementation will be used for ECDH and ECDHE
 * cipher suites, and for ECDSA support.
 *
 * \param cc    SSL engine context.
 * \param iec   EC implementation (or `NULL`).
 */
static inline void
br_ssl_engine_set_ec(br_ssl_engine_context *cc, const br_ec_impl *iec)
{
	cc->iec = iec;
}

/**
 * \brief Set the I/O buffer for the SSL engine.
 *
 * Once this call has been made, `br_ssl_client_reset()` or
 * `br_ssl_server_reset()` MUST be called before using the context.
 *
 * The provided buffer will be used as long as the engine context is
 * used. The caller is responsible for keeping it available.
 *
 * If `bidi` is 0, then the engine will operate in half-duplex mode
 * (it won't be able to send data while there is unprocessed incoming
 * data in the buffer, and it won't be able to receive data while there
 * is unsent data in the buffer). The optimal buffer size in half-duplex
 * mode is `BR_SSL_BUFSIZE_MONO`; if the buffer is larger, then extra
 * bytes are ignored. If the buffer is smaller, then this limits the
 * capacity of the engine to support all allowed record sizes.
 *
 * If `bidi` is 1, then the engine will split the buffer into two
 * parts, for separate handling of outgoing and incoming data. This
 * enables full-duplex processing, but requires more RAM. The optimal
 * buffer size in full-duplex mode is `BR_SSL_BUFSIZE_BIDI`; if the
 * buffer is larger, then extra bytes are ignored. If the buffer is
 * smaller, then the split will favour the incoming part, so that
 * interoperability is maximised.
 *
 * \param cc          SSL engine context
 * \param iobuf       I/O buffer.
 * \param iobuf_len   I/O buffer length (in bytes).
 * \param bidi        non-zero for full-duplex mode.
 */
void br_ssl_engine_set_buffer(br_ssl_engine_context *cc,
	void *iobuf, size_t iobuf_len, int bidi);

/**
 * \brief Set the I/O buffers for the SSL engine.
 *
 * Once this call has been made, `br_ssl_client_reset()` or
 * `br_ssl_server_reset()` MUST be called before using the context.
 *
 * This function is similar to `br_ssl_engine_set_buffer()`, except
 * that it enforces full-duplex mode, and the two I/O buffers are
 * provided as separate chunks.
 *
 * The macros `BR_SSL_BUFSIZE_INPUT` and `BR_SSL_BUFSIZE_OUTPUT`
 * evaluate to the optimal (maximum) sizes for the input and output
 * buffer, respectively.
 *
 * \param cc         SSL engine context
 * \param ibuf       input buffer.
 * \param ibuf_len   input buffer length (in bytes).
 * \param obuf       output buffer.
 * \param obuf_len   output buffer length (in bytes).
 */
void br_ssl_engine_set_buffers_bidi(br_ssl_engine_context *cc,
	void *ibuf, size_t ibuf_len, void *obuf, size_t obuf_len);

/**
 * \brief Inject some "initial entropy" in the context.
 *
 * This entropy will be added to what can be obtained from the
 * underlying operating system, if that OS is supported.
 *
 * This function may be called several times; all injected entropy chunks
 * are cumulatively mixed.
 *
 * If entropy gathering from the OS is supported and compiled in, then this
 * step is optional. Otherwise, it is mandatory to inject randomness, and
 * the caller MUST take care to push (as one or several successive calls)
 * enough entropy to achieve cryptographic resistance (at least 80 bits,
 * preferably 128 or more). The engine will report an error if no entropy
 * was provided and none can be obtained from the OS.
 *
 * Take care that this function cannot assess the cryptographic quality of
 * the provided bytes.
 *
 * In all generality, "entropy" must here be considered to mean "that
 * which the attacker cannot predict". If your OS/architecture does not
 * have a suitable source of randomness, then you can make do with the
 * combination of a large enough secret value (possibly a copy of an
 * asymmetric private key that you also store on the system) AND a
 * non-repeating value (e.g. current time, provided that the local clock
 * cannot be reset or altered by the attacker).
 *
 * \param cc     SSL engine context.
 * \param data   extra entropy to inject.
 * \param len    length of the extra data (in bytes).
 */
void br_ssl_engine_inject_entropy(br_ssl_engine_context *cc,
	const void *data, size_t len);

/**
 * \brief Get the "server name" in this engine.
 *
 * For clients, this is the name provided with `br_ssl_client_reset()`;
 * for servers, this is the name received from the client as part of the
 * ClientHello message. If there is no such name (e.g. the client did
 * not send an SNI extension) then the returned string is empty
 * (returned pointer points to a byte of value 0).
 *
 * The returned pointer refers to a buffer inside the context, which may
 * be overwritten as part of normal SSL activity (even within the same
 * connection, if a renegotiation occurs).
 *
 * \param cc   SSL engine context.
 * \return  the server name (possibly empty).
 */
static inline const char *
br_ssl_engine_get_server_name(const br_ssl_engine_context *cc)
{
	return cc->server_name;
}

/**
 * \brief Get the protocol version.
 *
 * This function returns the protocol version that is used by the
 * engine. That value is set after sending (for a server) or receiving
 * (for a client) the ServerHello message.
 *
 * \param cc   SSL engine context.
 * \return  the protocol version.
 */
static inline unsigned
br_ssl_engine_get_version(const br_ssl_engine_context *cc)
{
	return cc->session.version;
}

/**
 * \brief Get a copy of the session parameters.
 *
 * The session parameters are filled during the handshake, so this
 * function shall not be called before completion of the handshake.
 * The initial handshake is completed when the context first allows
 * application data to be injected.
 *
 * This function copies the current session parameters into the provided
 * structure. Beware that the session parameters include the master
 * secret, which is sensitive data, to handle with great care.
 *
 * \param cc   SSL engine context.
 * \param pp   destination structure for the session parameters.
 */
static inline void
br_ssl_engine_get_session_parameters(const br_ssl_engine_context *cc,
	br_ssl_session_parameters *pp)
{
	memcpy(pp, &cc->session, sizeof *pp);
}

/**
 * \brief Set the session parameters to the provided values.
 *
 * This function is meant to be used in the client, before doing a new
 * handshake; a session resumption will be attempted with these
 * parameters. In the server, this function has no effect.
 *
 * \param cc   SSL engine context.
 * \param pp   source structure for the session parameters.
 */
static inline void
br_ssl_engine_set_session_parameters(br_ssl_engine_context *cc,
	const br_ssl_session_parameters *pp)
{
	memcpy(&cc->session, pp, sizeof *pp);
}

/**
 * \brief Get the current engine state.
 *
 * An SSL engine (client or server) has, at any time, a state which is
 * the combination of zero, one or more of these flags:
 *
 *   - `BR_SSL_CLOSED`
 *
 *     Engine is finished, no more I/O (until next reset).
 *
 *   - `BR_SSL_SENDREC`
 *
 *     Engine has some bytes to send to the peer.
 *
 *   - `BR_SSL_RECVREC`
 *
 *     Engine expects some bytes from the peer.
 *
 *   - `BR_SSL_SENDAPP`
 *
 *     Engine may receive application data to send (or flush).
 *
 *   - `BR_SSL_RECVAPP`
 *
 *     Engine has obtained some application data from the peer,
 *     that should be read by the caller.
 *
 * If no flag at all is set (state value is 0), then the engine is not
 * fully initialised yet.
 *
 * The `BR_SSL_CLOSED` flag is exclusive; when it is set, no other flag
 * is set. To distinguish between a normal closure and an error, use
 * `br_ssl_engine_last_error()`.
 *
 * Generally speaking, `BR_SSL_SENDREC` and `BR_SSL_SENDAPP` are mutually
 * exclusive: the input buffer, at any point, either accumulates
 * plaintext data, or contains an assembled record that is being sent.
 * Similarly, `BR_SSL_RECVREC` and `BR_SSL_RECVAPP` are mutually exclusive.
 * This may change in a future library version.
 *
 * \param cc   SSL engine context.
 * \return  the current engine state.
 */
unsigned br_ssl_engine_current_state(const br_ssl_engine_context *cc);

/** \brief SSL engine state: closed or failed. */
#define BR_SSL_CLOSED    0x0001
/** \brief SSL engine state: record data is ready to be sent to the peer. */
#define BR_SSL_SENDREC   0x0002
/** \brief SSL engine state: engine may receive records from the peer. */
#define BR_SSL_RECVREC   0x0004
/** \brief SSL engine state: engine may accept application data to send. */
#define BR_SSL_SENDAPP   0x0008
/** \brief SSL engine state: engine has received application data. */
#define BR_SSL_RECVAPP   0x0010

/**
 * \brief Get the engine error indicator.
 *
 * The error indicator is `BR_ERR_OK` (0) if no error was encountered
 * since the last call to `br_ssl_client_reset()` or
 * `br_ssl_server_reset()`. Other status values are "sticky": they
 * remain set, and prevent all I/O activity, until cleared. Only the
 * reset calls clear the error indicator.
 *
 * \param cc   SSL engine context.
 * \return  0, or a non-zero error code.
 */
static inline int
br_ssl_engine_last_error(const br_ssl_engine_context *cc)
{
	return cc->err;
}

/*
 * There are four I/O operations, each identified by a symbolic name:
 *
 *   sendapp   inject application data in the engine
 *   recvapp   retrieving application data from the engine
 *   sendrec   sending records on the transport medium
 *   recvrec   receiving records from the transport medium
 *
 * Terminology works thus: in a layered model where the SSL engine sits
 * between the application and the network, "send" designates operations
 * where bytes flow from application to network, and "recv" for the
 * reverse operation. Application data (the plaintext that is to be
 * conveyed through SSL) is "app", while encrypted records are "rec".
 * Note that from the SSL engine point of view, "sendapp" and "recvrec"
 * designate bytes that enter the engine ("inject" operation), while
 * "recvapp" and "sendrec" designate bytes that exit the engine
 * ("extract" operation).
 *
 * For the operation 'xxx', two functions are defined:
 *
 *   br_ssl_engine_xxx_buf
 *      Returns a pointer and length to the buffer to use for that
 *      operation. '*len' is set to the number of bytes that may be read
 *      from the buffer (extract operation) or written to the buffer
 *      (inject operation). If no byte may be exchanged for that operation
 *      at that point, then '*len' is set to zero, and NULL is returned.
 *      The engine state is unmodified by this call.
 *
 *   br_ssl_engine_xxx_ack
 *      Informs the engine that 'len' bytes have been read from the buffer
 *      (extract operation) or written to the buffer (inject operation).
 *      The 'len' value MUST NOT be zero. The 'len' value MUST NOT exceed
 *      that which was obtained from a preceeding br_ssl_engine_xxx_buf()
 *      call.
 */

/**
 * \brief Get buffer for application data to send.
 *
 * If the engine is ready to accept application data to send to the
 * peer, then this call returns a pointer to the buffer where such
 * data shall be written, and its length is written in `*len`.
 * Otherwise, `*len` is set to 0 and `NULL` is returned.
 *
 * \param cc    SSL engine context.
 * \param len   receives the application data output buffer length, or 0.
 * \return  the application data output buffer, or `NULL`.
 */
unsigned char *br_ssl_engine_sendapp_buf(
	const br_ssl_engine_context *cc, size_t *len);

/**
 * \brief Inform the engine of some new application data.
 *
 * After writing `len` bytes in the buffer returned by
 * `br_ssl_engine_sendapp_buf()`, the application shall call this
 * function to trigger any relevant processing. The `len` parameter
 * MUST NOT be 0, and MUST NOT exceed the value obtained in the
 * `br_ssl_engine_sendapp_buf()` call.
 *
 * \param cc    SSL engine context.
 * \param len   number of bytes pushed (not zero).
 */
void br_ssl_engine_sendapp_ack(br_ssl_engine_context *cc, size_t len);

/**
 * \brief Get buffer for received application data.
 *
 * If the engine has received application data from the peer, hen this
 * call returns a pointer to the buffer from where such data shall be
 * read, and its length is written in `*len`. Otherwise, `*len` is set
 * to 0 and `NULL` is returned.
 *
 * \param cc    SSL engine context.
 * \param len   receives the application data input buffer length, or 0.
 * \return  the application data input buffer, or `NULL`.
 */
unsigned char *br_ssl_engine_recvapp_buf(
	const br_ssl_engine_context *cc, size_t *len);

/**
 * \brief Acknowledge some received application data.
 *
 * After reading `len` bytes from the buffer returned by
 * `br_ssl_engine_recvapp_buf()`, the application shall call this
 * function to trigger any relevant processing. The `len` parameter
 * MUST NOT be 0, and MUST NOT exceed the value obtained in the
 * `br_ssl_engine_recvapp_buf()` call.
 *
 * \param cc    SSL engine context.
 * \param len   number of bytes read (not zero).
 */
void br_ssl_engine_recvapp_ack(br_ssl_engine_context *cc, size_t len);

/**
 * \brief Get buffer for record data to send.
 *
 * If the engine has prepared some records to send to the peer, then this
 * call returns a pointer to the buffer from where such data shall be
 * read, and its length is written in `*len`. Otherwise, `*len` is set
 * to 0 and `NULL` is returned.
 *
 * \param cc    SSL engine context.
 * \param len   receives the record data output buffer length, or 0.
 * \return  the record data output buffer, or `NULL`.
 */
unsigned char *br_ssl_engine_sendrec_buf(
	const br_ssl_engine_context *cc, size_t *len);

/**
 * \brief Acknowledge some sent record data.
 *
 * After reading `len` bytes from the buffer returned by
 * `br_ssl_engine_sendrec_buf()`, the application shall call this
 * function to trigger any relevant processing. The `len` parameter
 * MUST NOT be 0, and MUST NOT exceed the value obtained in the
 * `br_ssl_engine_sendrec_buf()` call.
 *
 * \param cc    SSL engine context.
 * \param len   number of bytes read (not zero).
 */
void br_ssl_engine_sendrec_ack(br_ssl_engine_context *cc, size_t len);

/**
 * \brief Get buffer for incoming records.
 *
 * If the engine is ready to accept records from the peer, then this
 * call returns a pointer to the buffer where such data shall be
 * written, and its length is written in `*len`. Otherwise, `*len` is
 * set to 0 and `NULL` is returned.
 *
 * \param cc    SSL engine context.
 * \param len   receives the record data input buffer length, or 0.
 * \return  the record data input buffer, or `NULL`.
 */
unsigned char *br_ssl_engine_recvrec_buf(
	const br_ssl_engine_context *cc, size_t *len);

/**
 * \brief Inform the engine of some new record data.
 *
 * After writing `len` bytes in the buffer returned by
 * `br_ssl_engine_recvrec_buf()`, the application shall call this
 * function to trigger any relevant processing. The `len` parameter
 * MUST NOT be 0, and MUST NOT exceed the value obtained in the
 * `br_ssl_engine_recvrec_buf()` call.
 *
 * \param cc    SSL engine context.
 * \param len   number of bytes pushed (not zero).
 */
void br_ssl_engine_recvrec_ack(br_ssl_engine_context *cc, size_t len);

/**
 * \brief Flush buffered application data.
 *
 * If some application data has been buffered in the engine, then wrap
 * it into a record and mark it for sending. If no application data has
 * been buffered but the engine would be ready to accept some, AND the
 * `force` parameter is non-zero, then an empty record is assembled and
 * marked for sending. In all other cases, this function does nothing.
 *
 * Empty records are technically legal, but not all existing SSL/TLS
 * implementations support them. Empty records can be useful as a
 * transparent "keep-alive" mechanism to maintain some low-level
 * network activity.
 *
 * \param cc      SSL engine context.
 * \param force   non-zero to force sending an empty record.
 */
void br_ssl_engine_flush(br_ssl_engine_context *cc, int force);

/**
 * \brief Initiate a closure.
 *
 * If, at that point, the context is open and in ready state, then a
 * `close_notify` alert is assembled and marked for sending; this
 * triggers the closure protocol. Otherwise, no such alert is assembled.
 *
 * \param cc   SSL engine context.
 */
void br_ssl_engine_close(br_ssl_engine_context *cc);

/**
 * \brief Initiate a renegotiation.
 *
 * If the engine is failed or closed, or if the peer is known not to
 * support secure renegotiation (RFC 5746), or if renegotiations have
 * been disabled with the `BR_OPT_NO_RENEGOTIATION` flag, then this
 * function returns 0 and nothing else happens.
 *
 * Otherwise, this function returns 1, and a renegotiation attempt is
 * triggered (if a handshake is already ongoing at that point, then
 * no new handshake is triggered).
 *
 * \param cc   SSL engine context.
 * \return  1 on success, 0 on error.
 */
int br_ssl_engine_renegotiate(br_ssl_engine_context *cc);

/**
 * \brief Context structure for a SSL client.
 *
 * The first field (called `eng`) is the SSL engine; all functions that
 * work on a `br_ssl_engine_context` structure shall take as parameter
 * a pointer to that field. The other structure fields are opaque and
 * must not be accessed directly.
 */
typedef struct {
	/**
	 * \brief The encapsulated engine context.
	 */
	br_ssl_engine_context eng;

#ifndef BR_DOXYGEN_IGNORE
	/*
	 * Minimum ClientHello length; padding with an extension (RFC
	 * 7685) is added if necessary to match at least that length.
	 * Such padding is nominally unnecessary, but it has been used
	 * to work around some server implementation bugs.
	 */
	uint16_t min_clienthello_len;

	/*
	 * Implementations.
	 */
	br_rsa_public irsapub;
	br_rsa_pkcs1_vrfy irsavrfy;
	br_ecdsa_vrfy iecdsa;
#endif
} br_ssl_client_context;

/*
 * Each br_ssl_client_init_xxx() function sets the list of supported
 * cipher suites and used implementations, as specified by the profile
 * name 'xxx'. Defined profile names are:
 *
 *    full    all supported versions and suites; constant-time implementations
 *    TODO: add other profiles
 */

/**
 * \brief SSL client profile: full.
 *
 * This function initialises the provided SSL client context with
 * all supported algorithms and cipher suites. It also initialises
 * a companion X.509 validation engine with all supported algorithms,
 * and the provided trust anchors; the X.509 engine will be used by
 * the client context to validate the server's certificate.
 *
 * \param cc                  client context to initialise.
 * \param xc                  X.509 validation context to initialise.
 * \param trust_anchors       trust anchors to use.
 * \param trust_anchors_num   number of trust anchors.
 */
void br_ssl_client_init_full(br_ssl_client_context *cc,
	br_x509_minimal_context *xc,
	const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num);

/**
 * \brief Clear the complete contents of a SSL client context.
 *
 * Everything is cleared, including the reference to the configured buffer,
 * implementations, cipher suites and state. This is a preparatory step
 * to assembling a custom profile.
 *
 * \param cc   client context to clear.
 */
void br_ssl_client_zero(br_ssl_client_context *cc);

/**
 * \brief Set the RSA public-key operations implementation.
 *
 * This will be used to encrypt the pre-master secret with the server's
 * RSA public key (RSA-encryption cipher suites only).
 *
 * \param cc        client context.
 * \param irsapub   RSA public-key encryption implementation.
 */
static inline void
br_ssl_client_set_rsapub(br_ssl_client_context *cc, br_rsa_public irsapub)
{
	cc->irsapub = irsapub;
}

/**
 * \brief Set the RSA signature verification implementation.
 *
 * This will be used to verify the server's signature on its
 * ServerKeyExchange message (ECDHE_RSA cipher suites only).
 *
 * \param cc         client context.
 * \param irsavrfy   RSA signature verification implementation.
 */
static inline void
br_ssl_client_set_rsavrfy(br_ssl_client_context *cc, br_rsa_pkcs1_vrfy irsavrfy)
{
	cc->irsavrfy = irsavrfy;
}

/*
 * \brief Set the ECDSA implementation (signature verification).
 *
 * The ECDSA implementation will use the EC core implementation configured
 * in the engine context.
 *
 * \param cc       client context.
 * \param iecdsa   ECDSA verification implementation.
 */
static inline void
br_ssl_client_set_ecdsa(br_ssl_client_context *cc, br_ecdsa_vrfy iecdsa)
{
	cc->iecdsa = iecdsa;
}

/**
 * \brief Set the minimum ClientHello length (RFC 7685 padding).
 *
 * If this value is set and the ClientHello would be shorter, then
 * the Pad ClientHello extension will be added with enough padding bytes
 * to reach the target size. Because of the extension header, the resulting
 * size will sometimes be slightly more than `len` bytes if the target
 * size cannot be exactly met.
 *
 * The target length relates to the _contents_ of the ClientHello, not
 * counting its 4-byte header. For instance, if `len` is set to 512,
 * then the padding will bring the ClientHello size to 516 bytes with its
 * header, and 521 bytes when counting the 5-byte record header.
 *
 * \param cc    client context.
 * \param len   minimum ClientHello length (in bytes).
 */
static inline void
br_ssl_client_set_min_clienthello_len(br_ssl_client_context *cc, uint16_t len)
{
	cc->min_clienthello_len = len;
}

/**
 * \brief Prepare or reset a client context for a new connection.
 *
 * The `server_name` parameter is used to fill the SNI extension; the
 * X.509 "minimal" engine will also match that name against the server
 * names included in the server's certificate. If the parameter is
 * `NULL` then no SNI extension will be sent, and the X.509 "minimal"
 * engine (if used for server certificate validation) will not check
 * presence of any specific name in the received certificate.
 *
 * Therefore, setting the `server_name` to `NULL` shall be reserved
 * to cases where alternate or additional methods are used to ascertain
 * that the right server public key is used (e.g. a "known key" model).
 *
 * If `resume_session` is non-zero and the context was previously used
 * then the session parameters may be reused (depending on whether the
 * server previously sent a non-empty session ID, and accepts the session
 * resumption). The session parameters for session resumption can also
 * be set explicitly with `br_ssl_engine_set_session_parameters()`.
 *
 * On failure, the context is marked as failed, and this function
 * returns 0. A possible failure condition is when no initial entropy
 * was injected, and none could be obtained from the OS (either OS
 * randomness gathering is not supported, or it failed).
 *
 * \param cc               client context.
 * \param server_name      target server name, or `NULL`.
 * \param resume_session   non-zero to try session resumption.
 * \return  0 on failure, 1 on success.
 */
int br_ssl_client_reset(br_ssl_client_context *cc,
	const char *server_name, int resume_session);

/**
 * \brief Forget any session in the context.
 *
 * This means that the next handshake that uses this context will
 * necessarily be a full handshake (this applies both to new connections
 * and to renegotiations).
 *
 * \param cc   client context.
 */
static inline void
br_ssl_client_forget_session(br_ssl_client_context *cc)
{
	cc->eng.session.session_id_len = 0;
}

/**
 * \brief Type for a "translated cipher suite", as an array of two
 * 16-bit integers.
 *
 * The first element is the cipher suite identifier (as used on the wire).
 * The second element is the concatenation of four 4-bit elements which
 * characterise the cipher suite contents. In most to least significant
 * order, these 4-bit elements are:
 *
 *   - Bits 12 to 15: key exchange + server key type
 *
 *     | val | symbolic constant        | suite type  | details                                          |
 *     | :-- | :----------------------- | :---------- | :----------------------------------------------- |
 *     |  0  | `BR_SSLKEYX_RSA`         | RSA         | RSA key exchange, key is RSA (encryption)        |
 *     |  1  | `BR_SSLKEYX_ECDHE_RSA`   | ECDHE_RSA   | ECDHE key exchange, key is RSA (signature)       |
 *     |  2  | `BR_SSLKEYX_ECDHE_ECDSA` | ECDHE_ECDSA | ECDHE key exchange, key is EC (signature)        |
 *     |  3  | `BR_SSLKEYX_ECDH_RSA`    | ECDH_RSA    | Key is EC (key exchange), cert signed with RSA   |
 *     |  4  | `BR_SSLKEYX_ECDH_ECDSA`  | ECDH_ECDSA  | Key is EC (key exchange), cert signed with ECDSA |
 *
 *   - Bits 8 to 11: symmetric encryption algorithm
 *
 *     | val | symbolic constant      | symmetric encryption | key strength (bits) |
 *     | :-- | :--------------------- | :------------------- | :------------------ |
 *     |  0  | `BR_SSLENC_3DES_CBC`   | 3DES/CBC             | 168                 |
 *     |  1  | `BR_SSLENC_AES128_CBC` | AES-128/CBC          | 128                 |
 *     |  2  | `BR_SSLENC_AES256_CBC` | AES-256/CBC          | 256                 |
 *     |  3  | `BR_SSLENC_AES128_GCM` | AES-128/GCM          | 128                 |
 *     |  4  | `BR_SSLENC_AES256_GCM` | AES-256/GCM          | 256                 |
 *     |  5  | `BR_SSLENC_CHACHA20`   | ChaCha20/Poly1305    | 256                 |
 *
 *   - Bits 4 to 7: MAC algorithm
 *
 *     | val | symbolic constant  | MAC type     | details                               |
 *     | :-- | :----------------- | :----------- | :------------------------------------ |
 *     |  0  | `BR_SSLMAC_AEAD`   | AEAD         | No dedicated MAC (encryption is AEAD) |
 *     |  2  | `BR_SSLMAC_SHA1`   | HMAC/SHA-1   | Value matches `br_sha1_ID`            |
 *     |  4  | `BR_SSLMAC_SHA256` | HMAC/SHA-256 | Value matches `br_sha256_ID`          |
 *     |  5  | `BR_SSLMAC_SHA384` | HMAC/SHA-384 | Value matches `br_sha384_ID`          |
 *
 *   - Bits 0 to 3: hash function for PRF when used with TLS-1.2
 *
 *     | val | symbolic constant  | hash function | details                              |
 *     | :-- | :----------------- | :------------ | :----------------------------------- |
 *     |  4  | `BR_SSLPRF_SHA256` | SHA-256       | Value matches `br_sha256_ID`         |
 *     |  5  | `BR_SSLPRF_SHA384` | SHA-384       | Value matches `br_sha384_ID`         |
 *
 * For instance, cipher suite `TLS_RSA_WITH_AES_128_GCM_SHA256` has
 * standard identifier 0x009C, and is translated to 0x0304, for, in
 * that order: RSA key exchange (0), AES-128/GCM (3), AEAD integrity (0),
 * SHA-256 in the TLS PRF (4).
 */
typedef uint16_t br_suite_translated[2];

#ifndef BR_DOXYGEN_IGNORE
/*
 * Constants are already documented in the br_suite_translated type.
 */

#define BR_SSLKEYX_RSA           0
#define BR_SSLKEYX_ECDHE_RSA     1
#define BR_SSLKEYX_ECDHE_ECDSA   2
#define BR_SSLKEYX_ECDH_RSA      3
#define BR_SSLKEYX_ECDH_ECDSA    4

#define BR_SSLENC_3DES_CBC       0
#define BR_SSLENC_AES128_CBC     1
#define BR_SSLENC_AES256_CBC     2
#define BR_SSLENC_AES128_GCM     3
#define BR_SSLENC_AES256_GCM     4
#define BR_SSLENC_CHACHA20       5

#define BR_SSLMAC_AEAD           0
#define BR_SSLMAC_SHA1           br_sha1_ID
#define BR_SSLMAC_SHA256         br_sha256_ID
#define BR_SSLMAC_SHA384         br_sha384_ID

#define BR_SSLPRF_SHA256         br_sha256_ID
#define BR_SSLPRF_SHA384         br_sha384_ID

#endif

/*
 * Pre-declaration for the SSL server context.
 */
typedef struct br_ssl_server_context_ br_ssl_server_context;

/**
 * \brief Type for the server policy choices, taken after analysis of
 * the client message (ClientHello).
 */
typedef struct {
	/**
	 * \brief Cipher suite to use with that client.
	 */
	uint16_t cipher_suite;

	/**
	 * \brief Hash function for signing the ServerKeyExchange.
	 *
	 * This is the symbolic identifier for the hash function that
	 * will be used to sign the ServerKeyExchange message, for ECDHE
	 * cipher suites. This is ignored for RSA and ECDH cipher suites.
	 *
	 * Take care that with TLS 1.0 and 1.1, that value MUST match
	 * the protocol requirements: value must be 0 (MD5+SHA-1) for
	 * a RSA signature, or 2 (SHA-1) for an ECDSA signature. Only
	 * TLS 1.2 allows for other hash functions.
	 */
	int hash_id;

	/**
	 * \brief Certificate chain to send to the client.
	 *
	 * This is an array of `br_x509_certificate` objects, each
	 * normally containing a DER-encoded certificate. The server
	 * code does not try to decode these elements.
	 */
	const br_x509_certificate *chain;

	/**
	 * \brief Certificate chain length (number of certificates).
	 */
	size_t chain_len;
} br_ssl_server_choices;

/**
 * \brief Class type for a policy handler (server side).
 *
 * A policy handler selects the policy parameters for a connection
 * (cipher suite and other algorithms, and certificate chain to send to
 * the client); it also performs the server-side computations involving
 * its permanent private key.
 *
 * The SSL server engine will invoke first `choose()`, once the
 * ClientHello message has been received, then either `do_keyx()`
 * `do_sign()`, depending on the cipher suite.
 */
typedef struct br_ssl_server_policy_class_ br_ssl_server_policy_class;
struct br_ssl_server_policy_class_ {
	/**
	 * \brief Context size (in bytes).
	 */
	size_t context_size;

	/**
	 * \brief Select algorithms and certificates for this connection.
	 *
	 * This callback function shall fill the provided `choices`
	 * structure with the policy choices for this connection. This
	 * entails selecting the cipher suite, hash function for signing
	 * the ServerKeyExchange (applicable only to ECDHE cipher suites),
	 * and certificate chain to send.
	 *
	 * The callback receives a pointer to the server context that
	 * contains the relevant data. In particular, the functions
	 * `br_ssl_server_get_client_suites()`,
	 * `br_ssl_server_get_client_hashes()` and
	 * `br_ssl_server_get_client_curves()` can be used to obtain
	 * the cipher suites, hash functions and elliptic curves
	 * supported by both the client and server, respectively. The
	 * `br_ssl_engine_get_version()` and `br_ssl_engine_get_server_name()`
	 * functions yield the protocol version and requested server name
	 * (SNI), respectively.
	 *
	 * This function may modify its context structure (`pctx`) in
	 * arbitrary ways to keep track of its own choices.
	 *
	 * This function shall return 1 if appropriate policy choices
	 * could be made, or 0 if this connection cannot be pursued.
	 *
	 * \param pctx      policy context.
	 * \param cc        SSL server context.
	 * \param choices   destination structure for the policy choices.
	 * \return  1 on success, 0 on error.
	 */
	int (*choose)(const br_ssl_server_policy_class **pctx,
		const br_ssl_server_context *cc,
		br_ssl_server_choices *choices);

	/**
	 * \brief Perform key exchange (server part).
	 *
	 * This callback is invoked to perform the server-side cryptographic
	 * operation for a key exchange that is not ECDHE. This callback
	 * uses the private key.
	 *
	 * **For RSA key exchange**, the provided `data` (of length `len`
	 * bytes) shall be decrypted with the server's private key, and
	 * the 48-byte premaster secret copied back to the first 48 bytes
	 * of `data`.
	 *
	 *   - The caller makes sure that `len` is at least 59 bytes.
	 *
	 *   - This callback MUST check that the provided length matches
	 *     that of the key modulus; it shall report an error otherwise.
	 *
	 *   - If the length matches that of the RSA key modulus, then
	 *     processing MUST be constant-time, even if decryption fails,
	 *     or the padding is incorrect, or the plaintext message length
	 *     is not exactly 48 bytes.
	 *
	 *   - This callback needs not check the two first bytes of the
	 *     obtained pre-master secret (the caller will do that).
	 *
	 *   - If an error is reported (0), then what the callback put
	 *     in the first 48 bytes of `data` is unimportant (the caller
	 *     will use random bytes instead).
	 *
	 * **For ECDH key exchange**, the provided `data` (of length `len`
	 * bytes) is the elliptic curve point from the client. The
	 * callback shall multiply it with its private key, and store
	 * the resulting X coordinate in `data`, starting at offset 1
	 * (thus, simply encoding the point in compressed or uncompressed
	 * format in `data` is fine).
	 *
	 *   - If the input array does not have the proper length for
	 *     an encoded curve point, then an error (0) shall be reported.
	 *
	 *   - If the input array has the proper length, then processing
	 *     MUST be constant-time, even if the data is not a valid
	 *     encoded point.
	 *
	 *   - This callback MUST check that the input point is valid.
	 *
	 * Returned value is 1 on success, 0 on error.
	 *
	 * \param pctx   policy context.
	 * \param data   key exchange data from the client.
	 * \param len    key exchange data length (in bytes).
	 * \return  1 on success, 0 on error.
	 */
	uint32_t (*do_keyx)(const br_ssl_server_policy_class **pctx,
		unsigned char *data, size_t len);

	/**
	 * \brief Perform a signature (for a ServerKeyExchange message).
	 *
	 * This callback function is invoked for ECDHE cipher suites.
	 * On input, the hash value to sign is in `data`, of size
	 * `hv_len`; the involved hash function is identified by
	 * `hash_id`. The signature shall be computed and written
	 * back into `data`; the total size of that buffer is `len`
	 * bytes.
	 *
	 * This callback shall verify that the signature length does not
	 * exceed `len` bytes, and abstain from writing the signature if
	 * it does not fit.
	 *
	 * For RSA signatures, the `hash_id` may be 0, in which case
	 * this is the special header-less signature specified in TLS 1.0
	 * and 1.1, with a 36-byte hash value. Otherwise, normal PKCS#1
	 * v1.5 signatures shall be computed.
	 *
	 * Returned value is the signature length (in bytes), or 0 on error.
	 *
	 * \param pctx      policy context.
	 * \param hash_id   hash function identifier.
	 * \param hv_len    hash value length (in bytes).
	 * \param data      input/output buffer (hash value, then signature).
	 * \param len       total buffer length (in bytes).
	 * \return  signature length (in bytes) on success, or 0 on error.
	 */
	size_t (*do_sign)(const br_ssl_server_policy_class **pctx,
		int hash_id, size_t hv_len, unsigned char *data, size_t len);
};

/**
 * \brief A single-chain RSA policy handler.
 *
 * This policy context uses a single certificate chain, and a RSA
 * private key. The context can be restricted to only signatures or
 * only key exchange.
 *
 * Apart from the first field (vtable pointer), its contents are
 * opaque and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_ssl_server_policy_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	const br_x509_certificate *chain;
	size_t chain_len;
	const br_rsa_private_key *sk;
	unsigned allowed_usages;
	br_rsa_private irsacore;
	br_rsa_pkcs1_sign irsasign;
#endif
} br_ssl_server_policy_rsa_context;

/**
 * \brief A single-chain EC policy handler.
 *
 * This policy context uses a single certificate chain, and an EC
 * private key. The context can be restricted to only signatures or
 * only key exchange.
 *
 * Due to how TLS is defined, this context must be made aware whether
 * the server certificate was itself signed with RSA or ECDSA. The code
 * does not try to decode the certificate to obtain that information.
 *
 * Apart from the first field (vtable pointer), its contents are
 * opaque and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_ssl_server_policy_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	const br_x509_certificate *chain;
	size_t chain_len;
	const br_ec_private_key *sk;
	unsigned allowed_usages;
	unsigned cert_issuer_key_type;
	const br_multihash_context *mhash;
	const br_ec_impl *iec;
	br_ecdsa_sign iecdsa;
#endif
} br_ssl_server_policy_ec_context;

/**
 * \brief Class type for a session parameter cache.
 *
 * Session parameters are saved in the cache with `save()`, and
 * retrieved with `load()`. The cache implementation can apply any
 * storage and eviction strategy that it sees fit. The SSL server
 * context that performs the request is provided, so that its
 * functionalities may be used by the implementation (e.g. hash
 * functions or random number generation).
 */
typedef struct br_ssl_session_cache_class_ br_ssl_session_cache_class;
struct br_ssl_session_cache_class_ {
	/**
	 * \brief Context size (in bytes).
	 */
	size_t context_size;

	/**
	 * \brief Record a session.
	 *
	 * This callback should record the provided session parameters.
	 * The `params` structure is transient, so its contents shall
	 * be copied into the cache. The session ID has been randomly
	 * generated and always has length exactly 32 bytes.
	 *
	 * \param ctx          session cache context.
	 * \param server_ctx   SSL server context.
	 * \param params       session parameters to save.
	 */
	void (*save)(const br_ssl_session_cache_class **ctx,
		br_ssl_server_context *server_ctx,
		const br_ssl_session_parameters *params);

	/**
	 * \brief Lookup a session in the cache.
	 *
	 * The session ID to lookup is in `params` and always has length
	 * exactly 32 bytes. If the session parameters are found in the
	 * cache, then the parameters shall be copied into the `params`
	 * structure. Returned value is 1 on successful lookup, 0
	 * otherwise.
	 *
	 * \param ctx          session cache context.
	 * \param server_ctx   SSL server context.
	 * \param params       destination for session parameters.
	 * \return  1 if found, 0 otherwise.
	 */
	int (*load)(const br_ssl_session_cache_class **ctx,
		br_ssl_server_context *server_ctx,
		br_ssl_session_parameters *params);
};

/**
 * \brief Context for a basic cache system.
 *
 * The system stores session parameters in a buffer provided at
 * initialisation time. Each entry uses exactly 100 bytes, and
 * buffer sizes up to 4294967295 bytes are supported.
 *
 * Entries are evicted with a LRU (Least Recently Used) policy. A
 * search tree is maintained to keep lookups fast even with large
 * caches.
 *
 * Apart from the first field (vtable pointer), the structure
 * contents are opaque and shall not be accessed directly.
 */
typedef struct {
	/** \brief Pointer to vtable. */
	const br_ssl_session_cache_class *vtable;
#ifndef BR_DOXYGEN_IGNORE
	unsigned char *store;
	size_t store_len, store_ptr;
	unsigned char index_key[32];
	const br_hash_class *hash;
	int init_done;
	uint32_t head, tail, root;
#endif
} br_ssl_session_cache_lru;

/**
 * \brief Initialise a LRU session cache with the provided storage space.
 *
 * The provided storage space must remain valid as long as the cache
 * is used. Arbitrary lengths are supported, up to 4294967295 bytes;
 * each entry uses up exactly 100 bytes.
 *
 * \param cc          session cache context.
 * \param store       storage space for cached entries.
 * \param store_len   storage space length (in bytes).
 */
void br_ssl_session_cache_lru_init(br_ssl_session_cache_lru *cc,
	unsigned char *store, size_t store_len);

/**
 * \brief Context structure for a SSL server.
 *
 * The first field (called `eng`) is the SSL engine; all functions that
 * work on a `br_ssl_engine_context` structure shall take as parameter
 * a pointer to that field. The other structure fields are opaque and
 * must not be accessed directly.
 */
struct br_ssl_server_context_ {
	/**
	 * \brief The encapsulated engine context.
	 */
	br_ssl_engine_context eng;

#ifndef BR_DOXYGEN_IGNORE
	/*
	 * Maximum version from the client.
	 */
	uint16_t client_max_version;

	/*
	 * Session cache.
	 */
	const br_ssl_session_cache_class **cache_vtable;

	/*
	 * Translated cipher suites supported by the client. The list
	 * is trimmed to include only the cipher suites that the
	 * server also supports; they are in the same order as in the
	 * client message.
	 */
	br_suite_translated client_suites[BR_MAX_CIPHER_SUITES];
	unsigned char client_suites_num;

	/*
	 * Hash functions supported by the client, with ECDSA and RSA
	 * (bit mask). For hash function with id 'x', set bit index is
	 * x for RSA, x+8 for ECDSA.
	 */
	uint16_t hashes;

	/*
	 * Curves supported by the client (bit mask, for named curves).
	 */
	uint32_t curves;

	/*
	 * Context for chain handler.
	 */
	const br_ssl_server_policy_class **policy_vtable;
	const br_x509_certificate *chain;
	size_t chain_len;
	const unsigned char *cert_cur;
	size_t cert_len;
	unsigned char sign_hash_id;

	/*
	 * For the core handlers, thus avoiding (in most cases) the
	 * need for an externally provided policy context.
	 */
	union {
		const br_ssl_server_policy_class *vtable;
		br_ssl_server_policy_rsa_context single_rsa;
		br_ssl_server_policy_ec_context single_ec;
	} chain_handler;

	/*
	 * Buffer for the ECDHE private key.
	 */
	unsigned char ecdhe_key[70];
	size_t ecdhe_key_len;

	/*
	 * Server-specific implementations.
	 * (none for now)
	 */
#endif
};

/*
 * Each br_ssl_server_init_xxx() function sets the list of supported
 * cipher suites and used implementations, as specified by the profile
 * name 'xxx'. Defined profile names are:
 *
 *    full_rsa    all supported algorithm, server key type is RSA
 *    full_ec     all supported algorithm, server key type is EC
 *    TODO: add other profiles
 *
 * Naming scheme for "minimal" profiles: min123
 *
 * -- character 1: key exchange
 *      r = RSA
 *      e = ECDHE_RSA
 *      f = ECDHE_ECDSA
 *      u = ECDH_RSA
 *      v = ECDH_ECDSA
 * -- character 2: version / PRF
 *      0 = TLS 1.0 / 1.1 with MD5+SHA-1
 *      2 = TLS 1.2 with SHA-256
 *      3 = TLS 1.2 with SHA-384
 * -- character 3: encryption
 *      a = AES/CBC
 *      g = AES/GCM
 *      d = 3DES/CBC
 */

/**
 * \brief SSL server profile: full_rsa.
 *
 * This function initialises the provided SSL server context with
 * all supported algorithms and cipher suites that rely on a RSA
 * key pair.
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          RSA private key.
 */
void br_ssl_server_init_full_rsa(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk);

/**
 * \brief SSL server profile: full_ec.
 *
 * This function initialises the provided SSL server context with
 * all supported algorithms and cipher suites that rely on an EC
 * key pair.
 *
 * The key type of the CA that issued the server's certificate must
 * be provided, since it matters for ECDH cipher suites (ECDH_RSA
 * suites require a RSA-powered CA). The key type is either
 * `BR_KEYTYPE_RSA` or `BR_KEYTYPE_EC`.
 *
 * \param cc                     server context to initialise.
 * \param chain                  server certificate chain.
 * \param chain_len              chain length (number of certificates).
 * \param cert_issuer_key_type   certificate issuer's key type.
 * \param sk                     EC private key.
 */
void br_ssl_server_init_full_ec(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	unsigned cert_issuer_key_type, const br_ec_private_key *sk);

/**
 * \brief SSL server profile: minr2g.
 *
 * This profile uses only TLS_RSA_WITH_AES_128_GCM_SHA256. Server key is
 * RSA, and RSA key exchange is used (not forward secure, but uses little
 * CPU in the client).
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          RSA private key.
 */
void br_ssl_server_init_minr2g(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk);

/**
 * \brief SSL server profile: mine2g.
 *
 * This profile uses only TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256. Server key
 * is RSA, and ECDHE key exchange is used. This suite provides forward
 * security, with a higher CPU expense on the client, and a somewhat
 * larger code footprint (compared to "minr2g").
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          RSA private key.
 */
void br_ssl_server_init_mine2g(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk);

/**
 * \brief SSL server profile: minf2g.
 *
 * This profile uses only TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256.
 * Server key is EC, and ECDHE key exchange is used. This suite provides
 * forward security, with a higher CPU expense on the client and server
 * (by a factor of about 3 to 4), and a somewhat larger code footprint
 * (compared to "minu2g" and "minv2g").
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          EC private key.
 */
void br_ssl_server_init_minf2g(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk);

/**
 * \brief SSL server profile: minu2g.
 *
 * This profile uses only TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256.
 * Server key is EC, and ECDH key exchange is used; the issuing CA used
 * a RSA key.
 *
 * The "minu2g" and "minv2g" profiles do not provide forward secrecy,
 * but are the lightest on the server (for CPU usage), and are rather
 * inexpensive on the client as well.
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          EC private key.
 */
void br_ssl_server_init_minu2g(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk);

/**
 * \brief SSL server profile: minv2g.
 *
 * This profile uses only TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256.
 * Server key is EC, and ECDH key exchange is used; the issuing CA used
 * an EC key.
 *
 * The "minu2g" and "minv2g" profiles do not provide forward secrecy,
 * but are the lightest on the server (for CPU usage), and are rather
 * inexpensive on the client as well.
 *
 * \param cc          server context to initialise.
 * \param chain       server certificate chain.
 * \param chain_len   certificate chain length (number of certificate).
 * \param sk          EC private key.
 */
void br_ssl_server_init_minv2g(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk);

/**
 * \brief Get the supported client suites.
 *
 * This function shall be called only after the ClientHello has been
 * processed, typically from the policy engine. The returned array
 * contains the cipher suites that are supported by both the client
 * and the server; these suites are in client preference order, unless
 * the `BR_OPT_ENFORCE_SERVER_PREFERENCES` flag was set, in which case
 * they are in server preference order.
 *
 * The suites are _translated_, which means that each suite is given
 * as two 16-bit integers: the standard suite identifier, and its
 * translated version, broken down into its individual components,
 * as explained with the `br_suite_translated` type.
 *
 * The returned array is allocated in the context and will be rewritten
 * by each handshake.
 *
 * \param cc    server context.
 * \param num   receives the array size (number of suites).
 * \return  the translated common cipher suites, in preference order.
 */
static inline const br_suite_translated *
br_ssl_server_get_client_suites(const br_ssl_server_context *cc, size_t *num)
{
	*num = cc->client_suites_num;
	return cc->client_suites;
}

/**
 * \brief Get the hash functions supported by the client.
 *
 * This is a field of bits: for hash function of ID x, bit x is set if
 * the hash function is supported in RSA signatures, 8+x if it is supported
 * with ECDSA.
 *
 * \param cc   server context.
 * \return  the client-supported hash functions (for signatures).
 */
static inline uint16_t
br_ssl_server_get_client_hashes(const br_ssl_server_context *cc)
{
	return cc->hashes;
}

/**
 * \brief Get the elliptic curves supported by the client.
 *
 * This is a bit field (bit x is set if curve of ID x is supported).
 *
 * \param cc   server context.
 * \return  the client-supported elliptic curves.
 */
static inline uint32_t
br_ssl_server_get_client_curves(const br_ssl_server_context *cc)
{
	return cc->curves;
}

/**
 * \brief Clear the complete contents of a SSL server context.
 *
 * Everything is cleared, including the reference to the configured buffer,
 * implementations, cipher suites and state. This is a preparatory step
 * to assembling a custom profile.
 *
 * \param cc   server context to clear.
 */
void br_ssl_server_zero(br_ssl_server_context *cc);

/**
 * \brief Set an externally provided policy context.
 *
 * The policy context's methods are invoked to decide the cipher suite
 * and certificate chain, and to perform operations involving the server's
 * private key.
 *
 * \param cc     server context.
 * \param pctx   policy context (pointer to its vtable field).
 */
static inline void
br_ssl_server_set_policy(br_ssl_server_context *cc,
	const br_ssl_server_policy_class **pctx)
{
	cc->policy_vtable = pctx;
}

/**
 * \brief Set the server certificate chain and key (single RSA case).
 *
 * This function uses a policy context included in the server context.
 * It configures use of a single server certificate chain with a RSA
 * private key. The `allowed_usages` is a combination of usages, namely
 * `BR_KEYTYPE_KEYX` and/or `BR_KEYTYPE_SIGN`; this enables or disables
 * the corresponding cipher suites (i.e. `TLS_RSA_*` use the RSA key for
 * key exchange, while `TLS_ECDHE_RSA_*` use the RSA key for signatures).
 *
 * \param cc               server context.
 * \param chain            server certificate chain to send to the client.
 * \param chain_len        chain length (number of certificates).
 * \param sk               server private key (RSA).
 * \param allowed_usages   allowed private key usages.
 * \param irsacore         RSA core implementation.
 * \param irsasign         RSA signature implementation (PKCS#1 v1.5).
 */
void br_ssl_server_set_single_rsa(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_rsa_private_key *sk, unsigned allowed_usages,
	br_rsa_private irsacore, br_rsa_pkcs1_sign irsasign);

/*
 * \brief Set the server certificate chain and key (single EC case).
 *
 * This function uses a policy context included in the server context.
 * It configures use of a single server certificate chain with an EC
 * private key. The `allowed_usages` is a combination of usages, namely
 * `BR_KEYTYPE_KEYX` and/or `BR_KEYTYPE_SIGN`; this enables or disables
 * the corresponding cipher suites (i.e. `TLS_ECDH_*` use the EC key for
 * key exchange, while `TLS_ECDHE_ECDSA_*` use the EC key for signatures).
 *
 * In order to support `TLS_ECDH_*` cipher suites (non-ephemeral ECDH),
 * the algorithm type of the key used by the issuing CA to sign the
 * server's certificate must be provided, as `cert_issuer_key_type`
 * parameter (this value is either `BR_KEYTYPE_RSA` or `BR_KEYTYPE_EC`).
 *
 * \param cc                     server context.
 * \param chain                  server certificate chain to send.
 * \param chain_len              chain length (number of certificates).
 * \param sk                     server private key (EC).
 * \param allowed_usages         allowed private key usages.
 * \param cert_issuer_key_type   issuing CA's key type.
 * \param iec                    EC core implementation.
 * \param iecdsa                 ECDSA signature implementation ("asn1" format).
 */
void br_ssl_server_set_single_ec(br_ssl_server_context *cc,
	const br_x509_certificate *chain, size_t chain_len,
	const br_ec_private_key *sk, unsigned allowed_usages,
	unsigned cert_issuer_key_type,
	const br_ec_impl *iec, br_ecdsa_sign iecdsa);

/**
 * \brief Configure the cache for session parameters.
 *
 * The cache context is provided as a pointer to its first field (vtable
 * pointer).
 *
 * \param cc       server context.
 * \param vtable   session cache context.
 */
static inline void
br_ssl_server_set_cache(br_ssl_server_context *cc,
	const br_ssl_session_cache_class **vtable)
{
	cc->cache_vtable = vtable;
}

/**
 * \brief Prepare or reset a server context for handling an incoming client.
 *
 * \param cc   server context.
 * \return  1 on success, 0 on error.
 */
int br_ssl_server_reset(br_ssl_server_context *cc);

/* ===================================================================== */

/*
 * Context for the simplified I/O context. The transport medium is accessed
 * through the low_read() and low_write() callback functions, each with
 * its own opaque context pointer.
 *
 *  low_read()    read some bytes, at most 'len' bytes, into data[]. The
 *                returned value is the number of read bytes, or -1 on error.
 *                The 'len' parameter is guaranteed never to exceed 20000,
 *                so the length always fits in an 'int' on all platforms.
 *
 *  low_write()   write up to 'len' bytes, to be read from data[]. The
 *                returned value is the number of written bytes, or -1 on
 *                error. The 'len' parameter is guaranteed never to exceed
 *                20000, so the length always fits in an 'int' on all
 *                parameters.
 *
 * A socket closure (if the transport medium is a socket) should be reported
 * as an error (-1). The callbacks shall endeavour to block until at least
 * one byte can be read or written; a callback returning 0 at times is
 * acceptable, but this normally leads to the callback being immediately
 * called again, so the callback should at least always try to block for
 * some time if no I/O can take place.
 *
 * The SSL engine naturally applies some buffering, so the callbacks need
 * not apply buffers of their own.
 */
/**
 * \brief Context structure for the simplified SSL I/O wrapper.
 *
 * This structure is initialised with `br_sslio_init()`. Its contents
 * are opaque and shall not be accessed directly.
 */
typedef struct {
#ifndef BR_DOXYGEN_IGNORE
	br_ssl_engine_context *engine;
	int (*low_read)(void *read_context,
		unsigned char *data, size_t len);
	void *read_context;
	int (*low_write)(void *write_context,
		const unsigned char *data, size_t len);
	void *write_context;
#endif
} br_sslio_context;

/**
 * \brief Initialise a simplified I/O wrapper context.
 *
 * The simplified I/O wrapper offers a simpler read/write API for a SSL
 * engine (client or server), using the provided callback functions for
 * reading data from, or writing data to, the transport medium.
 *
 * The callback functions have the following semantics:
 *
 *   - Each callback receives an opaque context value (of type `void *`)
 *     that the callback may use arbitrarily (or possibly ignore).
 *
 *   - `low_read()` reads at least one byte, at most `len` bytes, from
 *     the transport medium. Read bytes shall be written in `data`.
 *
 *   - `low_write()` writes at least one byte, at most `len` bytes, unto
 *     the transport medium. The bytes to write are read from `data`.
 *
 *   - The `len` parameter is never zero, and is always lower than 20000.
 *
 *   - The number of processed bytes (read or written) is returned. Since
 *     that number is less than 20000, it always fits on an `int`.
 *
 *   - On error, the callbacks return -1. Reaching end-of-stream is an
 *     error. Errors are permanent: the SSL connection is terminated.
 *
 *   - Callbacks SHOULD NOT return 0. This is tolerated, as long as
 *     callbacks endeavour to block for some non-negligible amount of
 *     time until at least one byte can be sent or received (if a
 *     callback returns 0, then the wrapper invokes it again
 *     immediately).
 *
 *   - Callbacks MAY return as soon as at least one byte is processed;
 *     they MAY also insist on reading or writing _all_ requested bytes.
 *     Since SSL is a self-terminated protocol (each record has a length
 *     header), this does not change semantics.
 *
 *   - Callbacks need not apply any buffering (for performance) since SSL
 *     itself uses buffers.
 *
 * \param ctx             wrapper context to initialise.
 * \param engine          SSL engine to wrap.
 * \param low_read        callback for reading data from the transport.
 * \param read_context    context pointer for `low_read()`.
 * \param low_write       callback for writing data on the transport.
 * \param write_context   context pointer for `low_write()`.
 */
void br_sslio_init(br_sslio_context *ctx,
	br_ssl_engine_context *engine,
	int (*low_read)(void *read_context,
		unsigned char *data, size_t len),
	void *read_context,
	int (*low_write)(void *write_context,
		const unsigned char *data, size_t len),
	void *write_context);

/**
 * \brief Read some application data from a SSL connection.
 *
 * If `len` is zero, then this function returns 0 immediately. In
 * all other cases, it never returns 0.
 *
 * This call returns only when at least one byte has been obtained.
 * Returned value is the number of bytes read, or -1 on error. The
 * number of bytes always fits on an 'int' (data from a single SSL/TLS
 * record is returned).
 *
 * On error or SSL closure, this function returns -1. The caller should
 * inspect the error status on the SSL engine to distinguish between
 * normal closure and error.
 *
 * \param cc    SSL wrapper context.
 * \param dst   destination buffer for application data.
 * \param len   maximum number of bytes to obtain.
 * \return  number of bytes obtained, or -1 on error.
 */
int br_sslio_read(br_sslio_context *cc, void *dst, size_t len);

/**
 * \brief Read application data from a SSL connection.
 *
 * This calls returns only when _all_ requested `len` bytes are read,
 * or an error is reached. Returned value is 0 on success, -1 on error.
 * A normal (verified) SSL closure before that many bytes are obtained
 * is reported as an error by this function.
 *
 * \param cc    SSL wrapper context.
 * \param dst   destination buffer for application data.
 * \param len   number of bytes to obtain.
 * \return  0 on success, or -1 on error.
 */
int br_sslio_read_all(br_sslio_context *cc, void *dst, size_t len);

/**
 * \brief Write some application data unto a SSL connection.
 *
 * If `len` is zero, then this function returns 0 immediately. In
 * all other cases, it never returns 0.
 *
 * This call returns only when at least one byte has been written.
 * Returned value is the number of bytes written, or -1 on error. The
 * number of bytes always fits on an 'int' (less than 20000).
 *
 * On error or SSL closure, this function returns -1. The caller should
 * inspect the error status on the SSL engine to distinguish between
 * normal closure and error.
 *
 * **Important:** SSL is buffered; a "written" byte is a byte that was
 * injected into the wrapped SSL engine, but this does not necessarily mean
 * that it has been scheduled for sending. Use `br_sslio_flush()` to
 * ensure that all pending data has been sent to the transport medium.
 *
 * \param cc    SSL wrapper context.
 * \param src   source buffer for application data.
 * \param len   maximum number of bytes to write.
 * \return  number of bytes written, or -1 on error.
 */
int br_sslio_write(br_sslio_context *cc, const void *src, size_t len);

/**
 * \brief Write application data unto a SSL connection.
 *
 * This calls returns only when _all_ requested `len` bytes have been
 * written, or an error is reached. Returned value is 0 on success, -1
 * on error. A normal (verified) SSL closure before that many bytes are
 * written is reported as an error by this function.
 *
 * **Important:** SSL is buffered; a "written" byte is a byte that was
 * injected into the wrapped SSL engine, but this does not necessarily mean
 * that it has been scheduled for sending. Use `br_sslio_flush()` to
 * ensure that all pending data has been sent to the transport medium.
 *
 * \param cc    SSL wrapper context.
 * \param src   source buffer for application data.
 * \param len   number of bytes to write.
 * \return  0 on success, or -1 on error.
 */
int br_sslio_write_all(br_sslio_context *cc, const void *src, size_t len);

/**
 * \brief Flush pending data.
 *
 * This call makes sure that any buffered application data in the
 * provided context (including the wrapped SSL engine) has been sent
 * to the transport medium (i.e. accepted by the `low_write()` callback
 * method). If there is no such pending data, then this function does
 * nothing (and returns a success, i.e. 0).
 *
 * If the underlying transport medium has its own buffers, then it is
 * up to the caller to ensure the corresponding flushing.
 *
 * Returned value is 0 on success, -1 on error.
 *
 * \param cc    SSL wrapper context.
 * \return  0 on success, or -1 on error.
 */
int br_sslio_flush(br_sslio_context *cc);

/**
 * \brief Close the SSL connection.
 *
 * This call runs the SSL closure protocol (sending a `close_notify`,
 * receiving the response `close_notify`). When it returns, the SSL
 * connection is finished. It is still up to the caller to manage the
 * possible transport-level termination, if applicable (alternatively,
 * the underlying transport stream may be reused for non-SSL messages).
 *
 * Returned value is 0 on success, -1 on error. A failure by the peer
 * to process the complete closure protocol (i.e. sending back the
 * `close_notify`) is an error.
 *
 * \param cc    SSL wrapper context.
 * \return  0 on success, or -1 on error.
 */
int br_sslio_close(br_sslio_context *cc);

/* ===================================================================== */

/*
 * Symbolic constants for cipher suites.
 */

/* From RFC 5246 */
#define BR_TLS_NULL_WITH_NULL_NULL                   0x0000
#define BR_TLS_RSA_WITH_NULL_MD5                     0x0001
#define BR_TLS_RSA_WITH_NULL_SHA                     0x0002
#define BR_TLS_RSA_WITH_NULL_SHA256                  0x003B
#define BR_TLS_RSA_WITH_RC4_128_MD5                  0x0004
#define BR_TLS_RSA_WITH_RC4_128_SHA                  0x0005
#define BR_TLS_RSA_WITH_3DES_EDE_CBC_SHA             0x000A
#define BR_TLS_RSA_WITH_AES_128_CBC_SHA              0x002F
#define BR_TLS_RSA_WITH_AES_256_CBC_SHA              0x0035
#define BR_TLS_RSA_WITH_AES_128_CBC_SHA256           0x003C
#define BR_TLS_RSA_WITH_AES_256_CBC_SHA256           0x003D
#define BR_TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA          0x000D
#define BR_TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA          0x0010
#define BR_TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA         0x0013
#define BR_TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA         0x0016
#define BR_TLS_DH_DSS_WITH_AES_128_CBC_SHA           0x0030
#define BR_TLS_DH_RSA_WITH_AES_128_CBC_SHA           0x0031
#define BR_TLS_DHE_DSS_WITH_AES_128_CBC_SHA          0x0032
#define BR_TLS_DHE_RSA_WITH_AES_128_CBC_SHA          0x0033
#define BR_TLS_DH_DSS_WITH_AES_256_CBC_SHA           0x0036
#define BR_TLS_DH_RSA_WITH_AES_256_CBC_SHA           0x0037
#define BR_TLS_DHE_DSS_WITH_AES_256_CBC_SHA          0x0038
#define BR_TLS_DHE_RSA_WITH_AES_256_CBC_SHA          0x0039
#define BR_TLS_DH_DSS_WITH_AES_128_CBC_SHA256        0x003E
#define BR_TLS_DH_RSA_WITH_AES_128_CBC_SHA256        0x003F
#define BR_TLS_DHE_DSS_WITH_AES_128_CBC_SHA256       0x0040
#define BR_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256       0x0067
#define BR_TLS_DH_DSS_WITH_AES_256_CBC_SHA256        0x0068
#define BR_TLS_DH_RSA_WITH_AES_256_CBC_SHA256        0x0069
#define BR_TLS_DHE_DSS_WITH_AES_256_CBC_SHA256       0x006A
#define BR_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256       0x006B
#define BR_TLS_DH_anon_WITH_RC4_128_MD5              0x0018
#define BR_TLS_DH_anon_WITH_3DES_EDE_CBC_SHA         0x001B
#define BR_TLS_DH_anon_WITH_AES_128_CBC_SHA          0x0034
#define BR_TLS_DH_anon_WITH_AES_256_CBC_SHA          0x003A
#define BR_TLS_DH_anon_WITH_AES_128_CBC_SHA256       0x006C
#define BR_TLS_DH_anon_WITH_AES_256_CBC_SHA256       0x006D

/* From RFC 4492 */
#define BR_TLS_ECDH_ECDSA_WITH_NULL_SHA              0xC001
#define BR_TLS_ECDH_ECDSA_WITH_RC4_128_SHA           0xC002
#define BR_TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA      0xC003
#define BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA       0xC004
#define BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA       0xC005
#define BR_TLS_ECDHE_ECDSA_WITH_NULL_SHA             0xC006
#define BR_TLS_ECDHE_ECDSA_WITH_RC4_128_SHA          0xC007
#define BR_TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA     0xC008
#define BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA      0xC009
#define BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA      0xC00A
#define BR_TLS_ECDH_RSA_WITH_NULL_SHA                0xC00B
#define BR_TLS_ECDH_RSA_WITH_RC4_128_SHA             0xC00C
#define BR_TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA        0xC00D
#define BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA         0xC00E
#define BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA         0xC00F
#define BR_TLS_ECDHE_RSA_WITH_NULL_SHA               0xC010
#define BR_TLS_ECDHE_RSA_WITH_RC4_128_SHA            0xC011
#define BR_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA       0xC012
#define BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA        0xC013
#define BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA        0xC014
#define BR_TLS_ECDH_anon_WITH_NULL_SHA               0xC015
#define BR_TLS_ECDH_anon_WITH_RC4_128_SHA            0xC016
#define BR_TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA       0xC017
#define BR_TLS_ECDH_anon_WITH_AES_128_CBC_SHA        0xC018
#define BR_TLS_ECDH_anon_WITH_AES_256_CBC_SHA        0xC019

/* From RFC 5288 */
#define BR_TLS_RSA_WITH_AES_128_GCM_SHA256           0x009C
#define BR_TLS_RSA_WITH_AES_256_GCM_SHA384           0x009D
#define BR_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256       0x009E
#define BR_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384       0x009F
#define BR_TLS_DH_RSA_WITH_AES_128_GCM_SHA256        0x00A0
#define BR_TLS_DH_RSA_WITH_AES_256_GCM_SHA384        0x00A1
#define BR_TLS_DHE_DSS_WITH_AES_128_GCM_SHA256       0x00A2
#define BR_TLS_DHE_DSS_WITH_AES_256_GCM_SHA384       0x00A3
#define BR_TLS_DH_DSS_WITH_AES_128_GCM_SHA256        0x00A4
#define BR_TLS_DH_DSS_WITH_AES_256_GCM_SHA384        0x00A5
#define BR_TLS_DH_anon_WITH_AES_128_GCM_SHA256       0x00A6
#define BR_TLS_DH_anon_WITH_AES_256_GCM_SHA384       0x00A7

/* From RFC 5289 */
#define BR_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256   0xC023
#define BR_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384   0xC024
#define BR_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256    0xC025
#define BR_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384    0xC026
#define BR_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256     0xC027
#define BR_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384     0xC028
#define BR_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256      0xC029
#define BR_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384      0xC02A
#define BR_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256   0xC02B
#define BR_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384   0xC02C
#define BR_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256    0xC02D
#define BR_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384    0xC02E
#define BR_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256     0xC02F
#define BR_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384     0xC030
#define BR_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256      0xC031
#define BR_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384      0xC032

/* From RFC 7905 */
#define BR_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256     0xCCA8
#define BR_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256   0xCCA9
#define BR_TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256       0xCCAA
#define BR_TLS_PSK_WITH_CHACHA20_POLY1305_SHA256           0xCCAB
#define BR_TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256     0xCCAC
#define BR_TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256       0xCCAD
#define BR_TLS_RSA_PSK_WITH_CHACHA20_POLY1305_SHA256       0xCCAE

/* From RFC 7507 */
#define BR_TLS_FALLBACK_SCSV                         0x5600

/*
 * Symbolic constants for alerts.
 */
#define BR_ALERT_CLOSE_NOTIFY                0
#define BR_ALERT_UNEXPECTED_MESSAGE         10
#define BR_ALERT_BAD_RECORD_MAC             20
#define BR_ALERT_RECORD_OVERFLOW            22
#define BR_ALERT_DECOMPRESSION_FAILURE      30
#define BR_ALERT_HANDSHAKE_FAILURE          40
#define BR_ALERT_BAD_CERTIFICATE            42
#define BR_ALERT_UNSUPPORTED_CERTIFICATE    43
#define BR_ALERT_CERTIFICATE_REVOKED        44
#define BR_ALERT_CERTIFICATE_EXPIRED        45
#define BR_ALERT_CERTIFICATE_UNKNOWN        46
#define BR_ALERT_ILLEGAL_PARAMETER          47
#define BR_ALERT_UNKNOWN_CA                 48
#define BR_ALERT_ACCESS_DENIED              49
#define BR_ALERT_DECODE_ERROR               50
#define BR_ALERT_DECRYPT_ERROR              51
#define BR_ALERT_PROTOCOL_VERSION           70
#define BR_ALERT_INSUFFICIENT_SECURITY      71
#define BR_ALERT_INTERNAL_ERROR             80
#define BR_ALERT_USER_CANCELED              90
#define BR_ALERT_NO_RENEGOTIATION          100
#define BR_ALERT_UNSUPPORTED_EXTENSION     110

#endif
