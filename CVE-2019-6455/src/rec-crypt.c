/* -*- mode: C -*-
 *
 *       File:         rec-crypt.c
 *       Date:         Fri Aug 26 19:50:51 2011
 *
 *       GNU recutils - Encryption routines
 *
 */

/* Copyright (C) 2011-2019 Jose E. Marchesi */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <string.h>
#include <gcrypt.h>
#include <crc.h>
#include <base64.h>

#include <rec.h>
#include <rec-utils.h>

/* Size of a block in AES128 */
#define AESV2_BLKSIZE 16
#define AESV2_KEYSIZE 16

#define SALT_SIZE 4

static bool
rec_field_encrypted_p (rec_field_t field)
{
  return ((strlen (rec_field_value (field)) > strlen (REC_ENCRYPTED_PREFIX))
          && (strncmp (rec_field_value (field), REC_ENCRYPTED_PREFIX,
                       strlen (REC_ENCRYPTED_PREFIX)) == 0));
}

bool
rec_encrypt (char   *in,
             size_t  in_size,
             const char   *password,
             char  **out,
             size_t *out_size)
{
  gcry_cipher_hd_t handler;
  size_t i;
  size_t password_size;
  char key[AESV2_KEYSIZE];
  char iv[AESV2_BLKSIZE];
  size_t padding;
  uint32_t crc;
  char *real_in;
  size_t real_in_size;

  /* Append four bytes to the input buffer, containing the CRC of its
     contents.  This will be used as a control token to determine
     whether the correct key is used in decryption.
  
     We store the integer always in little-endian. */
  
  crc = crc32 (in, in_size);
  
#if defined WORDS_BIGENDIAN
  crc = rec_endian_swap (crc);
#endif

  real_in_size = in_size + 4;
  real_in = malloc (real_in_size + 4);
  memcpy (real_in, in, real_in_size);
  memcpy (real_in + real_in_size - 4, &crc, 4);

  /* The size of the input buffer must be bigger than AESV2_BLKSIZE,
     and must contain an entire number of blocks.  We assure that by
     padding the buffer with \0 characters.  */

  if ((real_in_size % AESV2_BLKSIZE) != 0)
    padding = AESV2_BLKSIZE - (real_in_size % AESV2_BLKSIZE);
  else
    padding = 0;

  if (padding != 0)
    {
      real_in_size = real_in_size + padding;
      real_in = realloc (real_in, real_in_size);

      for (i = 0; i < padding; i++)
        real_in[real_in_size - i - 1] = '\0';
    }  

  /* Create the handler.  */
  if (gcry_cipher_open (&handler,
                        GCRY_CIPHER_AES128,
                        GCRY_CIPHER_MODE_CBC,
                        0) != GPG_ERR_NO_ERROR)
    return false;

  /* Set the key of the cypher.  */
  password_size = strlen (password);
  for (i = 0; i < AESV2_KEYSIZE; i++)
    key[i] = password[i % password_size];

  /* Set both the key and the IV vector.  */
  if (gcry_cipher_setkey (handler, key, AESV2_KEYSIZE)
      != GPG_ERR_NO_ERROR)
    {
      gcry_cipher_close (handler);
      return false;
    }

  gcry_create_nonce (iv, SALT_SIZE);

  for (i = SALT_SIZE; i < AESV2_BLKSIZE; i++)
      iv[i] = i;

  if (gcry_cipher_setiv (handler, iv, AESV2_BLKSIZE)
      != GPG_ERR_NO_ERROR)
    {
      gcry_cipher_close (handler);
      return false;
    }

  *out_size = real_in_size + SALT_SIZE;
  *out = malloc (*out_size);

  /* Append salt at the end of the output.  */
  memcpy (*out + real_in_size, iv, SALT_SIZE);

  /* Encrypt the data.  */
  if (gcry_cipher_encrypt (handler,
                           *out,
                           real_in_size,
                           real_in,
                           real_in_size) != 0)
    {
      /* Error.  */
      gcry_cipher_close (handler);
      return false;
    }

  /* Close the handler.  */
  gcry_cipher_close (handler);

  return true;
}

bool
rec_decrypt (char   *in,
             size_t  in_size,
             const char   *password,
             char  **out,
             size_t *out_size)
{
  gcry_cipher_hd_t handler;
  size_t i;
  size_t password_size;
  char key[AESV2_KEYSIZE];
  char iv[AESV2_BLKSIZE];
  size_t salt_size = 0;

  if (((in_size - SALT_SIZE) % AESV2_BLKSIZE) == 0)
    salt_size = SALT_SIZE;
  else if ((in_size % AESV2_BLKSIZE) != 0)
    return false;

  /* Create the handler.  */
  if (gcry_cipher_open (&handler,
                        GCRY_CIPHER_AES128,
                        GCRY_CIPHER_MODE_CBC,
                        0) != GPG_ERR_NO_ERROR)
    return false;

  /* Set the key of the cypher.  */
  password_size = strlen (password);
  for (i = 0; i < AESV2_KEYSIZE; i++)
    key[i] = password[i % password_size];

  /* Set both the key and the IV vector.  */
  if (gcry_cipher_setkey (handler, key, AESV2_KEYSIZE)
      != GPG_ERR_NO_ERROR)
    {
      printf ("error setting key\n");
      gcry_cipher_close (handler);
      return false;
    }

  /* Extract salt at the end of the output.  */
  memcpy (iv, in + in_size - salt_size, salt_size);

  for (i = salt_size; i < AESV2_BLKSIZE; i++)
    iv[i] = i;

  if (gcry_cipher_setiv (handler, iv, AESV2_BLKSIZE)
      != GPG_ERR_NO_ERROR)
    {
      gcry_cipher_close (handler);
      return false;
    }

  /* Decrypt the data.  */
  *out_size = in_size - salt_size;
  *out = malloc (*out_size);
  if (gcry_cipher_decrypt (handler,
                           *out,
                           *out_size,
                           in,
                           in_size - salt_size) != 0)
    {
      /* Error.  */
      gcry_cipher_close (handler);
      return false;
    }

  /* Make sure the decrypted data is ok by checking the CRC at the end
     of the sequence.  */

  if (strlen(*out) > 4)
    {
      uint32_t crc = 0;
      
      memcpy (&crc, *out + strlen(*out) - 4, 4);
#if defined WORDS_BIGENDIAN
      crc = rec_endian_swap (crc);
#endif

      if (crc32 (*out, strlen(*out) - 4) != crc)
        {
          gcry_cipher_close (handler);
          return false;
        }

      (*out)[strlen(*out) - 4] = '\0';
    }
  else
    {
      gcry_cipher_close (handler);
      return false;
    }

  /* Close the handler.  */
  gcry_cipher_close (handler);

  return true;
}

bool
rec_encrypt_record (rec_rset_t rset,
                    rec_record_t record,
                    const char *password)
{
  rec_field_t field;
  bool res;
  const char *field_name;
  rec_fex_t confidential_fields;
  size_t i, k, num_fields;

  res = true;

  if (rset)
    {
      confidential_fields = rec_rset_confidential (rset);
      for (i = 0; i < rec_fex_size (confidential_fields); i++)
        {
          field_name = rec_fex_elem_field_name (rec_fex_get (confidential_fields, i));

          num_fields = rec_record_get_num_fields_by_name (record, field_name);
          for (k = 0; k < num_fields; k++)
            {
              field = rec_record_get_field_by_name (record, field_name, k);
              if (field)
                {
                  res = rec_encrypt_field (field, password);
                  if (!res)
                    break;
                }
            }
        }
    }

  return res;
}

bool
rec_encrypt_field (rec_field_t field,
                   const char *password)
{
  char *field_value;
  char *field_value_encrypted;
  char *field_value_base64;
  size_t out_size, base64_size;
  char *aux;

  field_value = strdup (rec_field_value (field));
  if (!field_value)
    return false;

  /* Make sure the field is not already encrypted.  */
  if ((strlen (rec_field_value (field)) >= strlen (REC_ENCRYPTED_PREFIX))
      && (strncmp (rec_field_value (field), REC_ENCRYPTED_PREFIX,
                   strlen (REC_ENCRYPTED_PREFIX)) == 0))
    return true;

  if (!rec_encrypt (field_value,
                    strlen (field_value),
                    password,
                    &field_value_encrypted,
                    &out_size))
    return false;
  
  /* Encode the encrypted value into base64.  */

  base64_size = base64_encode_alloc (field_value_encrypted,
                                     out_size,
                                     &field_value_base64);
  base64_encode (field_value_encrypted,
                 out_size,
                 field_value_base64,
                 base64_size);

  /* Prepennd "encrypted-".  */
  aux = malloc (strlen (field_value_base64)
                + strlen (REC_ENCRYPTED_PREFIX) + 1);
  memcpy (aux,
          REC_ENCRYPTED_PREFIX,
          strlen (REC_ENCRYPTED_PREFIX));
  memcpy (aux + strlen (REC_ENCRYPTED_PREFIX),
          field_value_base64,
          strlen (field_value_base64));
  aux[strlen (field_value_base64)
      + strlen (REC_ENCRYPTED_PREFIX)] = '\0';
  free (field_value_base64);
  field_value_base64 = aux;
  
  /* Replace the value of the field.  */
  rec_field_set_value (field, field_value_base64);
  
  /* Free resources.  */
  free (field_value);
  free (field_value_encrypted);
  free (field_value_base64);

  return true;
}

bool
rec_decrypt_field (rec_field_t field,
                   const char *password)
{
  const char *field_value;
  char *base64_decoded;
  size_t base64_decoded_size;
  char *decrypted_value;
  size_t decrypted_value_size;

  /* Make sure the field is encrypted.  */
  if ((strlen (rec_field_value (field)) < strlen (REC_ENCRYPTED_PREFIX))
      || (strncmp (rec_field_value (field), REC_ENCRYPTED_PREFIX,
                   strlen (REC_ENCRYPTED_PREFIX)) != 0))
    return true;

  /* Skip the "encrypted-" prefix.  */
  field_value = rec_field_value (field) + strlen (REC_ENCRYPTED_PREFIX);

  /* Decode the Base64.  */

  if (base64_decode_alloc (field_value,
                           strlen(field_value),
                           &base64_decoded,
                           &base64_decoded_size))
    {
      base64_decode (field_value,
                     strlen(field_value),
                     base64_decoded,
                     &base64_decoded_size);
      
      /* Decrypt.  */

      if (rec_decrypt (base64_decoded,
                       base64_decoded_size,
                       password,
                       &decrypted_value,
                       &decrypted_value_size))
        rec_field_set_value (field, decrypted_value);

      /* Free resources.  */
      free (base64_decoded);
    }

  return true;
}

bool
rec_decrypt_record (rec_rset_t rset,
                    rec_record_t record,
                    const char *password)
{
  bool res = true;
  size_t i, num_fields, k;
  rec_field_t field;
  const char *field_name;
  rec_fex_t confidential_fields;

  if (rset)
    {
      confidential_fields = rec_rset_confidential (rset);
      for (i = 0; i < rec_fex_size (confidential_fields); i++)
        {
          field_name = rec_fex_elem_field_name (rec_fex_get (confidential_fields, i));

          num_fields = rec_record_get_num_fields_by_name (record, field_name);
          for (k = 0; k < num_fields; k++)
            {
              field = rec_record_get_field_by_name (record, field_name, k);
              if (field)
                {
                  res = rec_decrypt_field (field, password);
                  if (!res)
                    break;
                }
            }
        }
    }

  return res;
}

/* End of rec-crypt.c */
