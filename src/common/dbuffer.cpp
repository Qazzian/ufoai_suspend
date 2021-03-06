/**
 * @file
 * @brief A dbuffer is a dynamically sized buffer that stores arbitrary bytes
 * in a queue. It does not provide random access; characters may be
 * inserted only at the end and removed only from the beginning.
 */

#include "common.h"

dbuffer::dbuffer () : _length(0)
{
	reserve(INITIAL_CAPACITY);
}

dbuffer::~dbuffer ()
{
}

void dbuffer::add (const char *data, size_t len)
{
	insert(begin() + _length, data, data + len);
	_length += len;
}

/**
 * @brief Read data from a dbuffer
 * @param[out] data pointer to where the data should be copied
 * @param[in] len maximum number of bytes to copy
 * @return number of bytes copied
 *
 * @par
 * Copies up to @c len bytes into @c data
 *
 * @par
 * If the buffer does not contain at least @c len bytes, then as many
 * bytes as are present will be copied.
 */
size_t dbuffer::get (char *data, size_t len) const
{
	if (len > _length) {
		len = _length;
	}
	dbuffer::const_iterator copyEnd = begin() + len;
	std::copy(begin(), copyEnd, data);

	return len;
}

/**
 * @brief Read data from a dbuffer
 * @param[in] offset the offset in the source buffer where data should be copied from
 * @param[out] data pointer to where the data should be copied
 * @param[in] len maximum number of bytes to copy
 * @return number of bytes copied
 *
 * @par
 * Copies up to @c len bytes into @c data
 *
 * @par
 * If the buffer does not contain at least @c len bytes after offset,
 * then as many bytes as are present will be copied.
 */
size_t dbuffer::getAt (size_t offset, char *data, size_t len) const
{
	if (offset > _length)
		return 0;

	dbuffer::const_iterator copyBegin = begin() + offset;
	if (len > _length) {
		len = _length;
	}
	dbuffer::const_iterator copyEnd = copyBegin + len;
	std::copy(copyBegin, copyEnd, data);

	return len;
}

/**
 * @brief Deletes data from a dbuffer
 * @param[in] len number of bytes to delete
 * Deletes the given number of bytes from the start of the dbuffer
 */
size_t dbuffer::remove (size_t len)
{
	if (len > _length) {
		len = _length;
	}
	dbuffer::iterator eraseEnd = begin() + len;
	erase(begin(), eraseEnd);
	_length -= len;
	return len;

}

/**
 * @brief Read and delete data from a dbuffer
 * @param[out] data pointer to where the data should be copied
 * @param[in] len maximum number of bytes to copy
 * @return number of bytes copied
 *
 * @par
 * Copies up to @c len bytes into @c data, and removes them from the dbuffer
 *
 * @par
 * If the buffer does not contain at least @c len bytes, then as many
 * bytes as are present will be copied.
 *
 * @par
 * However many bytes are copied, exactly that many will be removed
 * from the start of the dbuffer.
 */
size_t dbuffer::extract (char *data, size_t len)
{
	len = get(data, len);
	remove(len);
	return len;
}

/**
 * @brief Allocate a dbuffer
 * @param[in] old the source buffer
 * @return the newly allocated buffer
 * Allocates a new dbuffer and initialises it to contain a copy of the
 * data in old
 */
dbuffer *dbuffer::dup () const
{
	dbuffer* d = new_dbuffer();
	*d = *this;
	return d;
}
