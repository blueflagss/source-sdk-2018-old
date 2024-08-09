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

template< class T, class M = c_utl_memory< T > >
class c_utl_stack {
public:
    // constructor, destructor
    c_utl_stack( int growSize = 0, int initSize = 0 );

    void CopyFrom( const c_utl_stack< T, M > &from );

    // element access
    T &operator[]( int i );
    T const &operator[]( int i ) const;
    T &Element( int i );
    T const &Element( int i ) const;

    // Gets the base address (can change when adding elements!)
    T *Base( );
    T const *Base( ) const;

    // Looks at the stack top
    T &Top( );
    T const &Top( ) const;

    // Size
    int Count( ) const;

    // Is element index valid?
    bool IsIdxValid( int i ) const;

    // Adds an element, uses default constructor
    int Push( );

    // Adds an element, uses copy constructor
    int Push( T const &src );

    // Pops the stack
    void Pop( );
    void Pop( T &oldTop );
    void PopMultiple( int num );

    // Makes sure we have enough memory allocated to store a requested # of elements
    void EnsureCapacity( int num );

    // Clears the stack, no deallocation
    void Clear( );

    // Memory deallocation
    void Purge( );

private:
    // Grows the stack allocation
    void GrowStack( );

    // For easier access to the elements through the debugger
    void ResetDbgInfo( );

    M m_Memory;
    int m_Size;

    // For easier access to the elements through the debugger
    T *m_pElements;
};

//-----------------------------------------------------------------------------
// For easier access to the elements through the debugger
//-----------------------------------------------------------------------------

template< class T, class M >
inline void c_utl_stack< T, M >::ResetDbgInfo( ) {
    m_pElements = m_Memory.Base( );
}

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------

template< class T, class M >
c_utl_stack< T, M >::c_utl_stack( int growSize, int initSize ) : m_Size( 0 ) {
    ResetDbgInfo( );
}


//-----------------------------------------------------------------------------
// copy into
//-----------------------------------------------------------------------------

template< class T, class M >
void c_utl_stack< T, M >::CopyFrom( const c_utl_stack< T, M > &from ) {
    //Purge( );
    EnsureCapacity( from.Count( ) );
    for ( int i = 0; i < from.Count( ); i++ ) {
        Push( from[ i ] );
    }
}

//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------

template< class T, class M >
inline T &c_utl_stack< T, M >::operator[]( int i ) {
    // assert(IsIdxValid(i));
    return m_Memory[ i ];
}

template< class T, class M >
inline T const &c_utl_stack< T, M >::operator[]( int i ) const {
    // assert(IsIdxValid(i));
    return m_Memory[ i ];
}

template< class T, class M >
inline T &c_utl_stack< T, M >::Element( int i ) {
    // assert(IsIdxValid(i));
    return m_Memory[ i ];
}

template< class T, class M >
inline T const &c_utl_stack< T, M >::Element( int i ) const {
    // assert(IsIdxValid(i));
    return m_Memory[ i ];
}

//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------

template< class T, class M >
inline T *c_utl_stack< T, M >::Base( ) {
    return m_Memory.Base( );
}

template< class T, class M >
inline T const *c_utl_stack< T, M >::Base( ) const {
    return m_Memory.Base( );
}

//-----------------------------------------------------------------------------
// Returns the top of the stack
//-----------------------------------------------------------------------------

template< class T, class M >
inline T &c_utl_stack< T, M >::Top( ) {
    // assert(m_Size > 0);
    return Element( m_Size - 1 );
}

template< class T, class M >
inline T const &c_utl_stack< T, M >::Top( ) const {
    // assert(m_Size > 0);
    return Element( m_Size - 1 );
}

//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------

template< class T, class M >
inline int c_utl_stack< T, M >::Count( ) const {
    return m_Size;
}

//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------

template< class T, class M >
inline bool c_utl_stack< T, M >::IsIdxValid( int i ) const {
    return ( i >= 0 ) && ( i < m_Size );
}

//-----------------------------------------------------------------------------
// Grows the stack
//-----------------------------------------------------------------------------

template< class T, class M >
void c_utl_stack< T, M >::GrowStack( ) {
    if ( m_Size >= m_Memory.NumAllocated( ) )
        m_Memory.Grow( );

    ++m_Size;

    ResetDbgInfo( );
}

//-----------------------------------------------------------------------------
// Makes sure we have enough memory allocated to store a requested # of elements
//-----------------------------------------------------------------------------

template< class T, class M >
void c_utl_stack< T, M >::EnsureCapacity( int num ) {
    m_Memory.EnsureCapacity( num );
    ResetDbgInfo( );
}

//-----------------------------------------------------------------------------
// Adds an element, uses default constructor
//-----------------------------------------------------------------------------

template< class T, class M >
int c_utl_stack< T, M >::Push( ) {
    GrowStack( );
    Construct( &Element( m_Size - 1 ) );
    return m_Size - 1;
}

//-----------------------------------------------------------------------------
// Adds an element, uses copy constructor
//-----------------------------------------------------------------------------

template< class T, class M >
int c_utl_stack< T, M >::Push( T const &src ) {
    GrowStack( );
    CopyConstruct( &Element( m_Size - 1 ), src );
    return m_Size - 1;
}

//-----------------------------------------------------------------------------
// Pops the stack
//-----------------------------------------------------------------------------

template< class T, class M >
void c_utl_stack< T, M >::Pop( ) {
    // assert(m_Size > 0);
    Destruct( &Element( m_Size - 1 ) );
    --m_Size;
}

template< class T, class M >
void c_utl_stack< T, M >::Pop( T &oldTop ) {
    // assert(m_Size > 0);
    oldTop = Top( );
    Pop( );
}

template< class T, class M >
void c_utl_stack< T, M >::PopMultiple( int num ) {
    // assert(m_Size >= num);
    for ( int i = 0; i < num; ++i )
        Destruct( &Element( m_Size - i - 1 ) );
    m_Size -= num;
}

//-----------------------------------------------------------------------------
// Element removal
//-----------------------------------------------------------------------------

template< class T, class M >
void c_utl_stack< T, M >::Clear( ) {
    for ( int i = m_Size; --i >= 0; )
        Destruct( &Element( i ) );

    m_Size = 0;
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
class CUtlSymbolTable;
class CUtlSymbolTableMT;

//-----------------------------------------------------------------------------
// This is a symbol, which is a easier way of dealing with strings.
//-----------------------------------------------------------------------------
typedef unsigned short UtlSymId_t;

#define UTL_INVAL_SYMBOL ( ( UtlSymId_t ) ~0 )

class c_utl_symbol {
public:
    // constructor, destructor
    c_utl_symbol( ) : m_Id( UTL_INVAL_SYMBOL ) {
    }
    c_utl_symbol( UtlSymId_t id ) : m_Id( id ) {
    }

    c_utl_symbol( c_utl_symbol const &sym ) : m_Id( sym.m_Id ) {
    }

    // operator=
    c_utl_symbol &operator=( c_utl_symbol const &src ) {
        m_Id = src.m_Id;
        return *this;
    }

    // operator==
    bool operator==( c_utl_symbol const &src ) const {
        return m_Id == src.m_Id;
    }

    // Is valid?
    bool IsValid( ) const {
        return m_Id != UTL_INVAL_SYMBOL;
    }

    // Gets at the symbol
    operator UtlSymId_t( ) const {
        return m_Id;
    }

protected:
    UtlSymId_t m_Id;

    /*
    // The standard global symbol table
    static CUtlSymbolTableMT* s_pSymbolTable;

    static bool s_bAllowStaticSymbolTable;

    friend class CCleanupUtlSymbolTable;*/
};