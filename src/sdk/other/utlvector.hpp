#pragma once
#include <cassert>
#include <globals.hpp>

#pragma warning( disable : 4100 )
#pragma warning( disable : 4514 )

namespace utils
{
    void *alloc( size_t sz );
    void *realloc( void *mem, size_t sz );
    void free_memory( void *mem );
} // namespace utils

template< class T, class I = int >
class c_utl_memory {
public:
    __forceinline T &operator[]( I i ) {
        return m_pMemory[ i ];
    }

    T *Base( ) {
        return m_pMemory;
    }

    static int CalcNewAllocationCount( int count, int size, int requested, int bytes ) {
        if ( size )
            count = ( ( 1 + ( ( requested - 1 ) / size ) ) * size );

        else {
            if ( !count )
                count = ( 31 + bytes ) / bytes;

            while ( count < requested )
                count *= 2;
        }

        return count;
    }

    int NumAllocated( ) const {
        return m_nAllocationCount;
    }

    __forceinline void Grow( int count = 1 ) {
        if ( IsExternallyAllocated( ) )
            return;

        int requested = m_nAllocationCount + count;
        int new_alloc_count = CalcNewAllocationCount( m_nAllocationCount, m_nGrowSize, requested, sizeof( T ) );

        if ( ( int ) ( I ) new_alloc_count < requested ) {
            if ( ( int ) ( I ) new_alloc_count == 0 && ( int ) ( I ) ( new_alloc_count - 1 ) >= requested )
                --new_alloc_count;

            else {
                if ( ( int ) ( I ) requested != requested )
                    return;

                while ( ( int ) ( I ) new_alloc_count < requested )
                    new_alloc_count = ( new_alloc_count + requested ) / 2;
            }
        }

        m_nAllocationCount = new_alloc_count;

        if ( m_pMemory )
            m_pMemory = ( T * ) utils::realloc( m_pMemory, m_nAllocationCount * sizeof( T ) );

        else
            m_pMemory = ( T * ) utils::alloc( m_nAllocationCount * sizeof( T ) );
    }

    __forceinline bool IsExternallyAllocated( ) const {
        return m_nGrowSize < 0;
    }

protected:
    T *m_pMemory;
    int m_nAllocationCount;
    int m_nGrowSize;
};

template< class T >
__forceinline T *Construct( T *pMemory ) {
    return ::new ( pMemory ) T;
}

template< class T >
__forceinline void Destruct( T *pMemory ) {
    pMemory->~T( );
}

template< class T, class A = c_utl_memory< T > >
class c_utl_vector {
    typedef A CAllocator;

public:
    __forceinline T &operator[]( int i ) {
        return m_Memory[ i ];
    }

    __forceinline T &Element( int i ) {
        return m_Memory[ i ];
    }

    __forceinline T *Base( ) {
        return m_Memory.Base( );
    }

    __forceinline int Count( ) const {
        return m_Size;
    }

    __forceinline void RemoveAll( ) {
        for ( int i = m_Size; --i >= 0; )
            Destruct( &Element( i ) );

        m_Size = 0;
    }

    __forceinline int AddToTail( ) {
        return InsertBefore( m_Size );
    }

    __forceinline int InsertBefore( int elem ) {
        GrowVector( );
        ShiftElementsRight( elem );
        Construct( &Element( elem ) );

        return elem;
    }

    __forceinline void GrowVector( int num = 1 ) {
        if ( m_Size + num > m_Memory.NumAllocated( ) )
            m_Memory.Grow( m_Size + num - m_Memory.NumAllocated( ) );

        m_Size += num;
        ResetDbgInfo( );
    }

    __forceinline void ShiftElementsRight( int elem, int num = 1 ) {
        int numToMove = m_Size - elem - num;
        if ( ( numToMove > 0 ) && ( num > 0 ) )
            memmove( &Element( elem + num ), &Element( elem ), numToMove * sizeof( T ) );
    }

    CAllocator m_Memory;
    int m_Size;
    T *m_pElements;

    __forceinline void ResetDbgInfo( ) {
        m_pElements = Base( );
    }
};

template< class T >
inline void RangeCheck( const T &value, int minValue, int maxValue ) {
#ifdef _DEBUG
    //extern bool g_bDoRangeChecks;
    //if ( ThreadInMainThread( ) && g_bDoRangeChecks )
    //{
    // Ignore the min/max stuff for now.. just make sure it's not a NAN.
    //	Assert( _finite( value ) );
    //}
    // :troll:
#endif
}

inline void RangeCheck( const vector_3d &value, int minValue, int maxValue ) {
#ifdef _DEBUG
    RangeCheck( value.x, minValue, maxValue );
    RangeCheck( value.y, minValue, maxValue );
    RangeCheck( value.z, minValue, maxValue );
#endif
}

template< class T, int minValue, int maxValue, int startValue >
class CRangeCheckedVar {
public:
    inline CRangeCheckedVar( ) {
        m_Val = startValue;
    }

    inline CRangeCheckedVar( const T &value ) {
        *this = value;
    }

    T GetRaw( ) const {
        return m_Val;
    }

    // Clamp the value to its limits. Interpolation code uses this after interpolating.
    inline void Clamp( ) {
        if ( m_Val < minValue )
            m_Val = minValue;
        else if ( m_Val > maxValue )
            m_Val = maxValue;
    }

    inline operator const T &( ) const {
        return m_Val;
    }

    inline CRangeCheckedVar< T, minValue, maxValue, startValue > &operator=( const T &value ) {
        RangeCheck( value, minValue, maxValue );
        m_Val = value;
        return *this;
    }

    inline CRangeCheckedVar< T, minValue, maxValue, startValue > &operator+=( const T &value ) {
        return ( *this = m_Val + value );
    }

    inline CRangeCheckedVar< T, minValue, maxValue, startValue > &operator-=( const T &value ) {
        return ( *this = m_Val - value );
    }

    inline CRangeCheckedVar< T, minValue, maxValue, startValue > &operator*=( const T &value ) {
        return ( *this = m_Val * value );
    }

    inline CRangeCheckedVar< T, minValue, maxValue, startValue > &operator/=( const T &value ) {
        return ( *this = m_Val / value );
    }

private:
    T m_Val;
};