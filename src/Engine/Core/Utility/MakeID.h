#pragma once

#include "Core/Template/NonCopyable.h"
#include "Core/Memory/IAllocator.h"

/*
Author:
	Emil Persson, A.K.A. Humus.
	http://www.humus.name
License:
	Public Domain
*/

//=============================================================================
SE_NAMESPACE_BEGIN

class MakeID final : NonCopyable
{
private:
	struct Range
	{
		uint16_t first;
		uint16_t last;
	};
public:
	MakeID(IAllocator &allocator, const uint16_t max_id = std::numeric_limits<uint16_t>::max())
		: m_allocator(allocator)
		, m_ranges(static_cast<Range*>(allocator.Reallocate(nullptr, 0, sizeof(Range), 1)))
	{
		// Start with a single range, from 0 to max allowed ID (specified)
		m_ranges[0].first = 0;
		m_ranges[0].last = max_id;
	}

	~MakeID()
	{
		m_allocator.Reallocate(m_ranges, 0, 0, 1);
	}

	bool CreateID(uint16_t &id)
	{
		if ( m_ranges[0].first <= m_ranges[0].last )
		{
			id = m_ranges[0].first;

			// If current range is full and there is another one, that will become the new current range
			if ( m_ranges[0].first == m_ranges[0].last && m_count > 1 )
			{
				destroyRange(0);
			}
			else
			{
				++m_ranges[0].first;
			}
			return true;
		}

		// No available ID left
		return false;
	}

	bool CreateRangeID(uint16_t &id, const uint16_t count)
	{
		uint16_t i = 0;
		do
		{
			const uint16_t range_count = 1u + m_ranges[i].last - m_ranges[i].first;
			if ( count <= range_count )
			{
				id = m_ranges[i].first;

				// If current range is full and there is another one, that will become the new current range
				if ( count == range_count && i + 1 < m_count )
				{
					destroyRange(i);
				}
				else
				{
					m_ranges[i].first += count;
				}
				return true;
			}
			++i;
		} while ( i < m_count );

		// No range of free IDs was large enough to create the requested continuous ID sequence
		return false;
	}

	bool DestroyID(const uint16_t id)
	{
		return DestroyRangeID(id, 1);
	}

	bool DestroyRangeID(const uint16_t id, const uint16_t count)
	{
		const uint16_t end_id = static_cast<uint16_t>(id + count);

		// Binary search of the range list
		uint16_t i0 = 0u;
		uint16_t i1 = m_count - 1u;

		for ( ;;)
		{
			const uint16_t i = (i0 + i1) / 2u;

			if ( id < m_ranges[i].first )
			{
				// Before current range, check if neighboring
				if ( end_id >= m_ranges[i].first )
				{
					if ( end_id != m_ranges[i].first )
						return false; // Overlaps a range of free IDs, thus (at least partially) invalid IDs

					// Neighbor id, check if neighboring previous range too
					if ( i > i0 && id - 1 == m_ranges[i - 1].last )
					{
						// Merge with previous range
						m_ranges[i - 1].last = m_ranges[i].last;
						destroyRange(i);
					}
					else
					{
						// Just grow range
						m_ranges[i].first = id;
					}
					return true;
				}
				else
				{
					// Non-neighbor id
					if ( i != i0 )
					{
						// Cull upper half of list
						i1 = i - 1u;
					}
					else
					{
						// Found our position in the list, insert the deleted range here
						insertRange(i);
						m_ranges[i].first = id;
						m_ranges[i].last = end_id - 1u;
						return true;
					}
				}
			}
			else if ( id > m_ranges[i].last )
			{
				// After current range, check if neighboring
				if ( id - 1 == m_ranges[i].last )
				{
					// Neighbor id, check if neighboring next range too
					if ( i < i1 && end_id == m_ranges[i + 1].first )
					{
						// Merge with next range
						m_ranges[i].last = m_ranges[i + 1].last;
						destroyRange(i + 1u);
					}
					else
					{
						// Just grow range
						m_ranges[i].last += count;
					}
					return true;
				}
				else
				{
					// Non-neighbor id
					if ( i != i1 )
					{
						// Cull bottom half of list
						i0 = i + 1u;
					}
					else
					{
						// Found our position in the list, insert the deleted range here
						insertRange(i + 1u);
						m_ranges[i + 1].first = id;
						m_ranges[i + 1].last = end_id - 1u;
						return true;
					}
				}
			}
			else
			{
				// Inside a free block, not a valid ID
				return false;
			}

		}
	}

	bool IsID(const uint16_t id) const
	{
		// Binary search of the range list
		uint16_t i0 = 0u;
		uint16_t i1 = m_count - 1u;

		for ( ;;)
		{
			const uint16_t i = (i0 + i1) / 2u;

			if ( id < m_ranges[i].first )
			{
				if ( i == i0 )
					return true;

				// Cull upper half of list
				i1 = i - 1u;
			}
			else if ( id > m_ranges[i].last )
			{
				if ( i == i1 )
					return true;

				// Cull bottom half of list
				i0 = i + 1u;
			}
			else
			{
				// Inside a free block, not a valid ID
				return false;
			}

		}
	}

	uint16_t GetAvailableIDs() const
	{
		uint16_t count = m_count;
		uint16_t i = 0;

		do
		{
			count += m_ranges[i].last - m_ranges[i].first;
			++i;
		} while ( i < m_count );

		return count;
	}

	uint16_t GetLargestContinuousRange() const
	{
		uint16_t max_count = 0;
		uint16_t i = 0;

		do
		{
			uint16_t count = m_ranges[i].last - m_ranges[i].first + 1u;
			if ( count > max_count )
				max_count = count;

			++i;
		} while ( i < m_count );

		return max_count;
	}

#if SE_DEBUG
	void PrintRanges() const
	{
		uint16_t i = 0;
		for ( ;;)
		{
			if ( m_ranges[i].first < m_ranges[i].last )
				printf("%u-%u", m_ranges[i].first, m_ranges[i].last);
			else if ( m_ranges[i].first == m_ranges[i].last )
				printf("%u", m_ranges[i].first);
			else
				printf("-");

			++i;
			if ( i >= m_count )
			{
				printf("\n");
				return;
			}

			printf(", ");
		}
	}
#endif


private:

	void insertRange(const uint16_t index)
	{
		if ( m_count >= m_capacity )
		{
			m_ranges = static_cast<Range *>(m_allocator.Reallocate(m_ranges, sizeof(Range) * m_capacity, (m_capacity + m_capacity) * sizeof(Range), 1));
			m_capacity += m_capacity;
		}

		::memmove(m_ranges + index + 1, m_ranges + index, (m_count - index) * sizeof(Range));
		++m_count;
	}

	void destroyRange(const uint16_t index)
	{
		--m_count;
		::memmove(m_ranges + index, m_ranges + index + 1, (m_count - index) * sizeof(Range));
	}

	IAllocator &m_allocator;
	Range *m_ranges; // Sorted array of ranges of free IDs
	uint16_t m_count = 1;    // Number of ranges in list
	uint16_t m_capacity = 1; // Total capacity of range list

};

SE_NAMESPACE_END
//=============================================================================