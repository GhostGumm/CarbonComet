#include "comet_buffer.h"
#include "cometd_internal.h"


cometd_buffer_t * cometd_buffer_create(cometd_buffer_t * buffer, size_t size) {
	if (buffer) {
		buffer->buffer = (char*)cometd_malloc_fn(size);
		buffer->writeCurrent = buffer->buffer;
		buffer->readCurrent = buffer->buffer;
		buffer->size = size;
	}
	return buffer;
}
void cometd_buffer_free(cometd_buffer_t * buffer) {
	if (buffer)
		cometd_free_fn(buffer->buffer);
}
int cometd_buffer_append(cometd_buffer_t * buffer, const void * data, size_t size) {
	if (buffer && (buffer->size - (buffer->writeCurrent - buffer->buffer) >= size)) {
		memcpy(buffer->writeCurrent, data, size);
		buffer->writeCurrent += size;
		//CMTD_TRACE_DEBUG("writing size : %zu bytes to buffer. Content is %.*s \n", size, (int)size, (char*)data);
		return 0;
	} else
		return -1;// error
}
int cometd_buffer_read(cometd_buffer_t * buffer, void * dest, size_t size) {
	CMTD_TRACE_IN
	if (buffer) {
		size_t available = buffer->writeCurrent - buffer->readCurrent;
		if (available) {
			size_t actualSize = size > available ? available : size;
			memcpy(dest, buffer->readCurrent, actualSize);
			buffer->readCurrent += actualSize;
			//CMTD_TRACE_DEBUG("size : %zu . Buffer can accept %zu. Content is %.*s \n", actualSize, size, (int)actualSize, (char*)dest);
			CMTD_RETURN(actualSize);
		}
	}
	CMTD_RETURN(0);
}
int cometd_buffer_read_seek(cometd_buffer_t * buffer, long int offset, int origin) {
	if (buffer) {
		if (origin == SEEK_SET) {
			if (offset <= buffer->size) {
				buffer->readCurrent = buffer->buffer + offset;
				return 0;
			}
			return 1;
		}
		// else not implemented
	}
	return 1;
}
void cometd_buffer_reset(cometd_buffer_t * buffer) {
	if (buffer) {
		buffer->writeCurrent = buffer->buffer;
		buffer->readCurrent = buffer->buffer;
	}
}
size_t cometd_buffer_readable_bytes(cometd_buffer_t * buffer) {
	return buffer ? buffer->writeCurrent - buffer->readCurrent : 0;
}

#define ARRAY_BASE_SIZE 4
void cometd_array_add(void * element, cometd_array_t** array) {
	CMTD_TRACE_IN
	if (!array || !element) {
		CMTD_TRACE_OUT
		return ;
	}
	if (!*array) {
		*array = (cometd_array_t*) cometd_zmalloc_fn(sizeof(cometd_array_t));
		if (!*array) return ;
		(*array)->array = (void **)cometd_zmalloc_fn(sizeof(void *) * (ARRAY_BASE_SIZE+1));
		(*array)->capacity = ARRAY_BASE_SIZE;
	}
	void** p = (*array)->array;
	while(*p) p++;
	int offset = p - (*array)->array;
	size_t oldCapacity = (*array)->capacity;
	if (offset >= oldCapacity) {
		(*array)->capacity = oldCapacity * 2;
		p = (void **)cometd_zmalloc_fn(sizeof(void *) * ((*array)->capacity+1));
		memcpy(p, (*array)->array, sizeof(void *) * oldCapacity);
		cometd_free_fn((*array)->array);
		(*array)->array = p;
	}
	(*array)->array[offset] = element;	
	CMTD_TRACE_OUT
}
void cometd_array_free(cometd_array_t** array, cometd_free elementFree) {
	if (!array || !*array)
		return;
	if (elementFree) {
		void** p = (*array)->array;
		while(*p)
			elementFree(*p++);
	}
	cometd_free_fn((*array)->array);
	cometd_free_fn(*array);
	*array = 0;
}
