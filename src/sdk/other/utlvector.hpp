#pragma once
#include <cassert>
#include <globals.hpp>

#pragma warning( disable : 4100 )
#pragma warning( disable : 4514 )

template< class T >
inline void Construct( T *pMemory ) {
    ::new ( pMemory ) T;
}

template< class T >
inline void CopyConstruct( T *pMemory, T const &src ) {
    ::new ( pMemory ) T( src );
}

template< class T >
inline void Destruct( T *pMemory ) {
    pMemory->~T( );
}

//-----------------------------------------------------------------------------
// The c_utl_memory class:
// A growable memory class which doubles in size by default.
//-----------------------------------------------------------------------------
template< class T >
class c_utl_memory {
public:
    // constructor, destructor
    c_utl_memory( int nGrowSize = 0, int nInitSize = 0 );
    c_utl_memory( T *pMemory, int numElements );
    ~c_utl_memory( );

    // element access
    T &operator[]( int i );
    T const &operator[]( int i ) const;
    T &Element( int i );
    T const &Element( int i ) const;

    // Can we use this index?
    bool IsIdxValid( int i ) const;

    // Gets the base address (can change when adding elements!)
    T *Base( );
    T const *Base( ) const;

    // Attaches the buffer to external memory....
    void SetExternalBuffer( T *pMemory, int numElements );

    // Size
    int NumAllocated( ) const;
    int Count( ) const;

    // Grows the memory, so that at least allocated + num elements are allocated
    void Grow( int num = 1 );

    // Memory deallocation
    void Purge( );

    // is the memory externally allocated?
    bool IsExternallyAllocated( ) const;

    // Set the size by which the memory grows
    void SetGrowSize( int size );

    enum {
        EXTERNAL_BUFFER_MARKER = -1,
    };

    T *m_pMemory;
    int m_nAllocationCount;
    int m_nGrowSize;
};

namespace utils
{
    namespace memory
    {
        void *alloc( size_t sz );
        void *realloc( void *mem, size_t sz );
        void free_memory( void *mem );
    }// namespace memory
}// namespace utils

#pragma warning( push )
#pragma warning( disable : 6308 )

//-----------------------------------------------------------------------------
// Grows the memory
//-----------------------------------------------------------------------------
template< class T >
void c_utl_memory< T >::Grow( int num ) {
    assert( num > 0 );

    if ( IsExternallyAllocated( ) ) {
        // Can't grow a buffer whose memory was externally allocated
        assert( 0 );
        return;
    }

    // Make sure we have at least nuwhy::gameAllocated + num allocations.
    // Use the grow rules specified for this memory (in m_nGrowSize)
    int nAllocationRequested = m_nAllocationCount + num;
    while ( m_nAllocationCount < nAllocationRequested ) {
        if ( m_nAllocationCount != 0 ) {
            if ( m_nGrowSize ) {
                m_nAllocationCount += m_nGrowSize;
            } else {
                m_nAllocationCount += m_nAllocationCount;
            }
        } else {
            // Compute an allocation which is at least as big as a cache line...
            m_nAllocationCount = ( 31 + sizeof( T ) ) / sizeof( T );
            assert( m_nAllocationCount != 0 );
        }
    }

    if ( m_pMemory ) {
        m_pMemory = ( T * ) utils::memory::realloc( m_pMemory, m_nAllocationCount * sizeof( T ) );
    } else {
        m_pMemory = ( T * ) utils::memory::alloc( m_nAllocationCount * sizeof( T ) );
    }
}

#pragma warning( pop )


//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
template< class T >
c_utl_memory< T >::c_utl_memory( int nGrowSize, int nInitAllocationCount ) : m_pMemory( 0 ),
                                                                             m_nAllocationCount( nInitAllocationCount ), m_nGrowSize( nGrowSize ) {
    //Assert((nGrowSize >= 0) && (nGrowSize != EXTERNAL_BUFFER_MARKER));
    if ( m_nAllocationCount ) {
        m_pMemory = ( T * ) utils::memory::alloc( m_nAllocationCount * sizeof( T ) );
    }
}

template< class T >
c_utl_memory< T >::c_utl_memory( T *pMemory, int numElements ) : m_pMemory( pMemory ),
                                                                 m_nAllocationCount( numElements ) {
    // Special marker indicating externally supplied memory
    m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}

template< class T >
c_utl_memory< T >::~c_utl_memory( ) {
    Purge( );
}

//-----------------------------------------------------------------------------
// Attaches the buffer to external memory....
//-----------------------------------------------------------------------------
template< class T >
void c_utl_memory< T >::SetExternalBuffer( T *pMemory, int numElements ) {
    // Blow away any existing allocated memory
    Purge( );

    m_pMemory = pMemory;
    m_nAllocationCount = numElements;

    // Indicate that we don't own the memory
    m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}

//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------
template< class T >
inline T &c_utl_memory< T >::operator[]( int i ) {
    //Assert(IsIdxValid(i));
    return m_pMemory[ i ];
}

template< class T >
inline T const &c_utl_memory< T >::operator[]( int i ) const {
    //Assert(IsIdxValid(i));
    return m_pMemory[ i ];
}

template< class T >
inline T &c_utl_memory< T >::Element( int i ) {
    //Assert(IsIdxValid(i));
    return m_pMemory[ i ];
}

template< class T >
inline T const &c_utl_memory< T >::Element( int i ) const {
    //Assert(IsIdxValid(i));
    return m_pMemory[ i ];
}

//-----------------------------------------------------------------------------
// is the memory externally allocated?
//-----------------------------------------------------------------------------
template< class T >
bool c_utl_memory< T >::IsExternallyAllocated( ) const {
    return m_nGrowSize == EXTERNAL_BUFFER_MARKER;
}

template< class T >
void c_utl_memory< T >::SetGrowSize( int nSize ) {
    //Assert((nSize >= 0) && (nSize != EXTERNAL_BUFFER_MARKER));
    m_nGrowSize = nSize;
}

//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------
template< class T >
inline T *c_utl_memory< T >::Base( ) {
    return m_pMemory;
}

template< class T >
inline T const *c_utl_memory< T >::Base( ) const {
    return m_pMemory;
}

//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------
template< class T >
inline int c_utl_memory< T >::NumAllocated( ) const {
    return m_nAllocationCount;
}

template< class T >
inline int c_utl_memory< T >::Count( ) const {
    return m_nAllocationCount;
}

//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------
template< class T >
inline bool c_utl_memory< T >::IsIdxValid( int i ) const {
    return ( i >= 0 ) && ( i < m_nAllocationCount );
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------
template< class T >
void c_utl_memory< T >::Purge( ) {
    if ( !IsExternallyAllocated( ) ) {
        if ( m_pMemory ) {
            utils::memory::free_memory( ( void * ) m_pMemory );
            m_pMemory = 0;
        }
        m_nAllocationCount = 0;
    }
}

template< class T >
class c_utl_vector {
public:
    typedef T ElemType_t;

    // constructor, destructor
    c_utl_vector( int growSize = 0, int initSize = 0 );
    c_utl_vector( T *pMemory, int numElements );
    ~c_utl_vector( );

    // Copy the array.
    c_utl_vector< T > &operator=( const c_utl_vector< T > &other );

    // element access
    T &operator[]( int i );
    T const &operator[]( int i ) const;
    T &Element( int i );
    T const &Element( int i ) const;

    // Gets the base address (can change when adding elements!)
    T *Base( );
    T const *Base( ) const;

    // Returns the number of elements in the vector
    // SIZE IS DEPRECATED!
    int Count( ) const;
    int Size( ) const;// don't use me!

    // Is element index valid?
    bool IsValidIndex( int i ) const;
    static int InvalidIndex( void );

    // Adds an element, uses default constructor
    int AddToHead( );
    int AddToTail( );
    int InsertBefore( int elem );
    int InsertAfter( int elem );

    // Adds an element, uses copy constructor
    int AddToHead( T const &src );
    int AddToTail( T const &src );
    int InsertBefore( int elem, T const &src );
    int InsertAfter( int elem, T const &src );

    // Adds multiple elements, uses default constructor
    int AddMultipleToHead( int num );
    int AddMultipleToTail( int num, const T *pToCopy = NULL );
    int InsertMultipleBefore( int elem, int num, const T *pToCopy = NULL );// If pToCopy is set, then it's an array of length 'num' and
    int InsertMultipleAfter( int elem, int num );

    // Calls RemoveAll() then AddMultipleToTail.
    void SetSize( int size );
    void SetCount( int count );

    // Calls SetSize and copies each element.
    void CopyArray( T const *pArray, int size );

    // Add the specified array to the tail.
    int AddVectorToTail( c_utl_vector< T > const &src );

    // Finds an element (element needs operator== defined)
    int Find( T const &src ) const;

    bool HasElement( T const &src );

    // Makes sure we have at least this many elements
    void EnsureCount( int num );

    // Element removal
    void FastRemove( int elem );       // doesn't preserve order
    void Remove( int elem );           // preserves order, shifts elements
    void FindAndRemove( T const &src );// removes first occurrence of src, preserves order, shifts elements
    bool FindAndFastRemove( const T &src );
    void RemoveMultiple( int elem, int num );// preserves order, shifts elements
    void RemoveAll( );                       // doesn't deallocate memory

    // Memory deallocation
    void Purge( );

    // Purges the list and calls delete on each element in it.
    void PurgeAndDeleteElements( );

    // Set the size by which it grows when it needs to allocate more memory.
    void SetGrowSize( int size );

protected:
    // Can't copy this unless we explicitly do it!
    c_utl_vector( c_utl_vector const &vec ) {
        assert( 0 );
    }

    // Grows the vector
    void GrowVector( int num = 1 );

    // Shifts elements....
    void ShiftElementsRight( int elem, int num = 1 );
    void ShiftElementsLeft( int elem, int num = 1 );

    // For easier access to the elements through the debugger
    void ResetDbgInfo( );

    c_utl_memory< T > m_Memory;
    int m_Size;

    // For easier access to the elements through the debugger
    // it's in release builds so this can be used in libraries correctly
    T *m_pElements;
};

//-----------------------------------------------------------------------------
// For easier access to the elements through the debugger
//-----------------------------------------------------------------------------

template< class T >
inline void c_utl_vector< T >::ResetDbgInfo( ) {
    m_pElements = m_Memory.Base( );
}

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------

template< class T >
inline c_utl_vector< T >::c_utl_vector( int growSize, int initSize ) : m_Memory( growSize, initSize ), m_Size( 0 ) {
    ResetDbgInfo( );
}

template< class T >
inline c_utl_vector< T >::c_utl_vector( T *pMemory, int numElements ) : m_Memory( pMemory, numElements ), m_Size( 0 ) {
    ResetDbgInfo( );
}

template< class T >
inline c_utl_vector< T >::~c_utl_vector( ) {
    Purge( );
}

template< class T >
inline c_utl_vector< T > &c_utl_vector< T >::operator=( const c_utl_vector< T > &other ) {
    CopyArray( other.Base( ), other.Count( ) );
    return *this;
}

//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------

template< class T >
inline T &c_utl_vector< T >::operator[]( int i ) {
    assert( IsValidIndex( i ) );
    return m_Memory[ i ];
}

template< class T >
inline T const &c_utl_vector< T >::operator[]( int i ) const {
    assert( IsValidIndex( i ) );
    return m_Memory[ i ];
}

template< class T >
inline T &c_utl_vector< T >::Element( int i ) {
    assert( IsValidIndex( i ) );
    return m_Memory[ i ];
}

template< class T >
inline T const &c_utl_vector< T >::Element( int i ) const {
    assert( IsValidIndex( i ) );
    return m_Memory[ i ];
}

//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------

template< class T >
inline T *c_utl_vector< T >::Base( ) {
    return m_Memory.Base( );
}

template< class T >
inline T const *c_utl_vector< T >::Base( ) const {
    return m_Memory.Base( );
}

//-----------------------------------------------------------------------------
// Count
//-----------------------------------------------------------------------------

template< class T >
inline int c_utl_vector< T >::Size( ) const {
    return m_Size;
}

template< class T >
inline int c_utl_vector< T >::Count( ) const {
    return m_Size;
}

//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------

template< class T >
inline bool c_utl_vector< T >::IsValidIndex( int i ) const {
    return ( i >= 0 ) && ( i < m_Size );
}

//-----------------------------------------------------------------------------
// Returns in invalid index
//-----------------------------------------------------------------------------
template< class T >
inline int c_utl_vector< T >::InvalidIndex( void ) {
    return -1;
}

//-----------------------------------------------------------------------------
// Grows the vector
//-----------------------------------------------------------------------------
template< class T >
void c_utl_vector< T >::GrowVector( int num ) {
    if ( m_Size + num - 1 >= m_Memory.NumAllocated( ) ) {
        m_Memory.Grow( m_Size + num - m_Memory.NumAllocated( ) );
    }

    m_Size += num;
    ResetDbgInfo( );
}

//-----------------------------------------------------------------------------
// Makes sure we have at least this many elements
//-----------------------------------------------------------------------------
template< class T >
void c_utl_vector< T >::EnsureCount( int num ) {
    if ( Count( ) < num )
        AddMultipleToTail( num - Count( ) );
}

//-----------------------------------------------------------------------------
// Shifts elements
//-----------------------------------------------------------------------------
template< class T >
void c_utl_vector< T >::ShiftElementsRight( int elem, int num ) {
    assert( IsValidIndex( elem ) || ( m_Size == 0 ) || ( num == 0 ) );
    int numToMove = m_Size - elem - num;
    if ( ( numToMove > 0 ) && ( num > 0 ) )
        memmove( &Element( elem + num ), &Element( elem ), numToMove * sizeof( T ) );
}

template< class T >
void c_utl_vector< T >::ShiftElementsLeft( int elem, int num ) {
    assert( IsValidIndex( elem ) || ( m_Size == 0 ) || ( num == 0 ) );
    int numToMove = m_Size - elem - num;
    if ( ( numToMove > 0 ) && ( num > 0 ) ) {
        memmove( &Element( elem ), &Element( elem + num ), numToMove * sizeof( T ) );

#ifdef _DEBUG
        memset( &Element( m_Size - num ), 0xDD, num * sizeof( T ) );
#endif
    }
}

//-----------------------------------------------------------------------------
// Adds an element, uses default constructor
//-----------------------------------------------------------------------------

template< class T >
inline int c_utl_vector< T >::AddToHead( ) {
    return InsertBefore( 0 );
}

template< class T >
inline int c_utl_vector< T >::AddToTail( ) {
    return InsertBefore( m_Size );
}

template< class T >
inline int c_utl_vector< T >::InsertAfter( int elem ) {
    return InsertBefore( elem + 1 );
}

template< class T >
int c_utl_vector< T >::InsertBefore( int elem ) {
    // Can insert at the end
    assert( ( elem == Count( ) ) || IsValidIndex( elem ) );

    GrowVector( );
    ShiftElementsRight( elem );
    Construct( &Element( elem ) );
    return elem;
}

//-----------------------------------------------------------------------------
// Adds an element, uses copy constructor
//-----------------------------------------------------------------------------

template< class T >
inline int c_utl_vector< T >::AddToHead( T const &src ) {
    return InsertBefore( 0, src );
}

template< class T >
inline int c_utl_vector< T >::AddToTail( T const &src ) {
    return InsertBefore( m_Size, src );
}

template< class T >
inline int c_utl_vector< T >::InsertAfter( int elem, T const &src ) {
    return InsertBefore( elem + 1, src );
}

template< class T >
int c_utl_vector< T >::InsertBefore( int elem, T const &src ) {
    // Can insert at the end
    assert( ( elem == Count( ) ) || IsValidIndex( elem ) );

    GrowVector( );
    ShiftElementsRight( elem );
    CopyConstruct( &Element( elem ), src );
    return elem;
}

//-----------------------------------------------------------------------------
// Adds multiple elements, uses default constructor
//-----------------------------------------------------------------------------

template< class T >
inline int c_utl_vector< T >::AddMultipleToHead( int num ) {
    return InsertMultipleBefore( 0, num );
}

template< class T >
inline int c_utl_vector< T >::AddMultipleToTail( int num, const T *pToCopy ) {
    return InsertMultipleBefore( m_Size, num, pToCopy );
}

template< class T >
int c_utl_vector< T >::InsertMultipleAfter( int elem, int num ) {
    return InsertMultipleBefore( elem + 1, num );
}

template< class T >
void c_utl_vector< T >::SetCount( int count ) {
    RemoveAll( );
    AddMultipleToTail( count );
}

template< class T >
inline void c_utl_vector< T >::SetSize( int size ) {
    SetCount( size );
}

template< class T >
void c_utl_vector< T >::CopyArray( T const *pArray, int size ) {
    SetSize( size );
    for ( int i = 0; i < size; i++ )
        ( *this )[ i ] = pArray[ i ];
}

template< class T >
int c_utl_vector< T >::AddVectorToTail( c_utl_vector const &src ) {
    int base = Count( );

    // Make space.
    AddMultipleToTail( src.Count( ) );

    // Copy the elements.
    for ( int i = 0; i < src.Count( ); i++ )
        ( *this )[ base + i ] = src[ i ];

    return base;
}

template< class T >
inline int c_utl_vector< T >::InsertMultipleBefore( int elem, int num, const T *pToInsert ) {
    if ( num == 0 )
        return elem;

    // Can insert at the end
    assert( ( elem == Count( ) ) || IsValidIndex( elem ) );

    GrowVector( num );
    ShiftElementsRight( elem, num );

    // Invoke default constructors
    for ( int i = 0; i < num; ++i )
        Construct( &Element( elem + i ) );

    // Copy stuff in?
    if ( pToInsert ) {
        for ( int i = 0; i < num; i++ ) {
            Element( elem + i ) = pToInsert[ i ];
        }
    }

    return elem;
}

//-----------------------------------------------------------------------------
// Finds an element (element needs operator== defined)
//-----------------------------------------------------------------------------
template< class T >
int c_utl_vector< T >::Find( T const &src ) const {
    for ( int i = 0; i < Count( ); ++i ) {
        if ( Element( i ) == src )
            return i;
    }
    return -1;
}

template< class T >
bool c_utl_vector< T >::HasElement( T const &src ) {
    return ( Find( src ) >= 0 );
}

//-----------------------------------------------------------------------------
// Element removal
//-----------------------------------------------------------------------------

template< class T >
void c_utl_vector< T >::FastRemove( int elem ) {
    assert( IsValidIndex( elem ) );

    Destruct( &Element( elem ) );
    if ( m_Size > 0 ) {
        Q_memcpy( &Element( elem ), &Element( m_Size - 1 ), sizeof( T ) );
        --m_Size;
    }
}

template< class T >
void c_utl_vector< T >::Remove( int elem ) {
    Destruct( &Element( elem ) );
    ShiftElementsLeft( elem );
    --m_Size;
}

template< class T >
void c_utl_vector< T >::FindAndRemove( T const &src ) {
    int elem = Find( src );
    if ( elem != -1 ) {
        Remove( elem );
    }
}

template< class T >
bool c_utl_vector< T >::FindAndFastRemove( const T &src ) {
    int elem = Find( src );
    if ( elem != -1 )
        ;
    {
        FastRemove( elem );
        return true;
    }
    return false;
}

template< class T >
void c_utl_vector< T >::RemoveMultiple( int elem, int num ) {
    assert( IsValidIndex( elem ) );
    assert( elem + num <= Count( ) );

    for ( int i = elem + num; --i >= elem; )
        Destruct( &Element( i ) );

    ShiftElementsLeft( elem, num );
    m_Size -= num;
}

template< class T >
void c_utl_vector< T >::RemoveAll( ) {
    for ( int i = m_Size; --i >= 0; )
        Destruct( &Element( i ) );

    m_Size = 0;
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------

template< class T >
void c_utl_vector< T >::Purge( ) {
    RemoveAll( );
    m_Memory.Purge( );
    ResetDbgInfo( );
}

template< class T >
inline void c_utl_vector< T >::PurgeAndDeleteElements( ) {
    for ( int i = 0; i < m_Size; i++ )
        delete Element( i );

    Purge( );
}

template< class T >
void c_utl_vector< T >::SetGrowSize( int size ) {
    m_Memory.SetGrowSize( size );
}

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