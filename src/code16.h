
#ifndef CODE16_H
#define CODE16_H

#if defined( __GNUC__ ) && defined( GCC_REALMODE )
asm( ".code16gcc" );
#endif /* __GNUC__ */

#endif /* CODE16_H */

