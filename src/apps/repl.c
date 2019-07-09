
#include "../kernel.h"
#include "../console.h"
#include "../mfat.h"
#include "../mbmp.h"
#include "../net/net.h"

#define CONSOLE_FLAG_INITIALIZED    0x01

/* Memory IDs for console tasks. */
#define REPL_MID_LINE      1
#define REPL_MID_CUR_POS   2
#define REPL_MID_ARG_MIN   10
/* Empty */
#define REPL_MID_ARG_MAX   20

#define REPL_LINE_SIZE_MAX 20

uint8_t g_console_flags = 0;

const char qd_logo[8][16] = {
   "     _____     ",
   "   .`_| |_`.   ",
   "  / /_| |_\\ \\  ",
   " |  __| |___|  ",
   " | |  | |      ",
   "  \\ \\_| |___   ",
   "   `._|_____/  ",
   "               "
};

__attribute__( (constructor) )
void repl_constructor() {
   adhd_launch_task( trepl_task );
}

/* = Command Callbacks */

#ifdef USE_NET

extern struct astring g_str_netp;

static TASK_RETVAL tnet_start( const struct astring* cli ) {
   tprintf( "start?\n" );
   adhd_launch_task( net_respond_task );
   return RETVAL_OK;
}

static TASK_RETVAL tnet_rcvd( const struct astring* cli ) {
   const int* received = NULL;
   TASK_PID pid;

   pid = adhd_get_pid_by_gid( &g_str_netp );
   received = mget( pid, NET_MID_RECEIVED, sizeof( int ) );
   tprintf( "frames rcvd: %d\n", *received );

   return RETVAL_OK;
}

#define NET_COMMANDS_COUNT 2
const struct api_command g_net_commands[NET_COMMANDS_COUNT] = {
   { "rcvd", tnet_rcvd },
   { "start", tnet_start },
};

static TASK_RETVAL trepl_net( const struct astring* cli ) {
   const char* tok;
   uint8_t i = 0;

   tok = alpha_tok( cli, ' ', 1 );
   if( NULL == tok ) {
      return 1;
   }

   for( i = 0 ; NET_COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp_cc(
         tok, CMD_MAX_LEN, g_net_commands[i].command, CMD_MAX_LEN, ' '
      ) ) {
         return g_net_commands[i].callback( cli );
      }
   }

   return RETVAL_BAD_ARGS;
}

#endif /* USE_NET */

static TASK_RETVAL tsys_exit( const struct astring* cli ) {
   g_system_state = SYSTEM_SHUTDOWN;
   return RETVAL_OK;
}

static TASK_RETVAL tsys_mem( const struct astring* cli ) {
   tprintf(
      "mem:\nused: %d\ntotal: %d\nfree: %d\n",
      get_mem_used(), MEM_HEAP_SIZE, MEM_HEAP_SIZE - get_mem_used()
   );
   return RETVAL_OK;
}

#define SYS_COMMANDS_COUNT 2
const struct api_command g_sys_commands[SYS_COMMANDS_COUNT] = {
   { "exit", tsys_exit },
   { "mem", tsys_mem }
};

static TASK_RETVAL trepl_sys( const struct astring* cli ) {
   const char* tok;
   uint8_t i = 0;

   tok = alpha_tok( cli, ' ', 1 );
   if( NULL == tok ) {
      return 1;
   }

   for( i = 0 ; SYS_COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp_cc(
         tok, CMD_MAX_LEN, g_sys_commands[i].command, CMD_MAX_LEN, ' '
      ) ) {
         return g_sys_commands[i].callback( cli );
      }
   }

   return RETVAL_BAD_ARGS;
}

#ifdef USE_DISK

static TASK_RETVAL tdisk_dir( const struct astring* cli ) {
   const char* tok;
   uint16_t offset = 0;
   char filename[13] = { 0 };
   uint8_t attrib = 0;
   char attrib_str[5] = { 0 };
   uint32_t size = 0;
   //uint16_t data_offset = 0;

   offset = mfat_get_root_dir_first_entry_offset( 0, 0 );

   tok = alpha_tok( cli, ' ', 2 );

   do {
      mfat_get_dir_entry_name( filename, offset, 0, 0 );

      if( NULL != tok ) {
         offset = mfat_get_dir_entry_offset( tok, 0, offset, 0, 0 );
         if( 0 == offset ) {
            break;
         }
         mfat_get_dir_entry_name( filename, offset, 0, 0 );
      }

      /* Convert the attribs to a printable format. */
      attrib = mfat_get_dir_entry_attrib( offset, 0, 0 );
      attrib_str[0] = MFAT_ATTRIB_RO == (MFAT_ATTRIB_RO & attrib) ? 'R' : ' ';
      attrib_str[1] = MFAT_ATTRIB_ARC == (MFAT_ATTRIB_ARC & attrib) ? 'A' : ' ';
      attrib_str[2] =
         MFAT_ATTRIB_SYSTEM == (MFAT_ATTRIB_SYSTEM & attrib) ? 'S' : ' ';
      attrib_str[3] = MFAT_ATTRIB_DIR == (MFAT_ATTRIB_DIR & attrib) ? 'D' : ' ';

      /* Get the file size. */
      size = mfat_get_dir_entry_size( offset, 0, 0 );

      //data_offset = mfat_get_dir_entry_first_cluster_offset( offset, 0, 0 );

      /* Print the entry. */
      tprintf( "- %s\t%12 s\t%d\t%d\n", attrib_str, filename,
         mfat_get_dir_entry_cyear( offset, 0, 0 ) + 1980, size );

      /* Get the next entry. */
      offset = mfat_get_dir_entry_next_offset( offset, 0, 0 );
   } while( 0 < offset );

   return RETVAL_OK;
}

static TASK_RETVAL tdisk_fat( const struct astring* cli ) {
   uint16_t i = 0;
   uint16_t entries_count = 0;
   uint16_t fat_entry = 0;
   const char* tok;
   uint8_t display_dec = 0;

   i = 2;
   tok = alpha_tok( cli, ' ', i );
   while( NULL != tok ) {
      if( !alpha_cmp_cc( "d", 1, tok, 1, ' ' ) ) {
         display_dec = 1;
      } else if( 0 < alpha_atou_c( tok, 5 /* TODO */, 10 ) ) {
         if( display_dec ) {
            /* Interpret as decimal. */
            entries_count = alpha_atou_c( tok, 5, 10 );
            fat_entry = mfat_get_fat_entry( entries_count, 0, 0 );
            tprintf( "fat entry %5d: %5d\n", entries_count, fat_entry );
            return RETVAL_OK;
         } else {
            /* Interpret as hex. */
            entries_count = alpha_atou_c( tok, 5, 16 );
            fat_entry = mfat_get_fat_entry( entries_count, 0, 0 );
            tprintf( "fat entry %4x: %4x\n", entries_count, fat_entry );
            return RETVAL_OK;
         }
      }
      i++;
      tok = alpha_tok( cli, ' ', i );
   }

   entries_count = mfat_get_entries_count( 0, 0 );

   tprintf( "\n00000\t" );
   for( i = 0 ; entries_count > i ; i++ ) {
      fat_entry = mfat_get_fat_entry( i, 0, 0 );

      if( display_dec ) {
         tprintf( "%5d ", fat_entry );
      } else {
         tprintf( "%4x ", fat_entry );
      }

      if( 0 == (i % 10) && 0 != i ) {
         if( display_dec ) {
            tprintf( "\n%5d\t", i );
         } else {
            tprintf( "\n%4x\t", i );
         }
      }
   }
   tprintf( "\n" );

   return RETVAL_OK;
}

static TASK_RETVAL tdisk_cat( const struct astring* cli ) {
   const char* tok;
   uint32_t offset = 0;
   char buffer[MFAT_FILENAME_LEN + 1] = { 0 };
   uint32_t written = 0;
   uint32_t file_size = 0;

   tok = alpha_tok( cli, ' ', 2 );
   if( NULL == tok ) {
      return RETVAL_BAD_ARGS;
   }

   offset = mfat_get_root_dir_first_entry_offset( 0, 0 );
   offset = mfat_get_dir_entry_offset( tok, MFAT_FILENAME_LEN, offset, 0, 0 );
   mfat_get_dir_entry_name( buffer, offset, 0, 0 );
   tprintf( "%s:\n\n", buffer );
   file_size = mfat_get_dir_entry_size( offset, 0, 0 );

   tprintf( "\"" );
   do {
      mzero( buffer, 11 );
      written += mfat_get_dir_entry_data( offset, written,
         (unsigned char*)buffer, 8, 0, 0 );
      tprintf( "%s", buffer );
   } while( file_size > written );
   tprintf( "\"\n" );

   return RETVAL_OK;
}

static TASK_RETVAL tdisk_bmp( const struct astring* cli ) {
   const char* tok;
   FILEPTR_T offset = 0;

   tok = alpha_tok( cli, ' ', 2 );
   if( NULL == tok ) {
      return RETVAL_BAD_ARGS;
   }

   if( 0 != (offset = mbmp_validate( tok, MFAT_FILENAME_LEN, 0, 0 )) ) {
      tprintf( "bitmap\nwidth: %d\nheight: %d\nbpp: %d\n",
         mbmp_get_width( offset, 0, 0 ),
         mbmp_get_height( offset, 0, 0 ),
         mbmp_get_bpp( offset, 0, 0 )
         );
   }

   return RETVAL_OK;
}

#define DISK_COMMANDS_COUNT 4
const struct api_command g_disk_commands[DISK_COMMANDS_COUNT] = {
   { "dir", tdisk_dir },
   { "fat", tdisk_fat },
   { "cat", tdisk_cat },
   { "bmp", tdisk_bmp }
};

static TASK_RETVAL trepl_disk( const struct astring* cli ) {
   const char* tok;
   uint8_t i = 0;

   tok = alpha_tok( cli, ' ', 1 );
   if( NULL == tok ) {
      return 1;
   }

   for( i = 0 ; DISK_COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp_cc(
         tok, CMD_MAX_LEN, g_disk_commands[i].command, CMD_MAX_LEN, ' '
      ) ) {
         return g_disk_commands[i].callback( cli );
      }
   }

   return RETVAL_BAD_ARGS;
}

#endif /* USE_DISK */

/**
 * Map typed commands to callbacks.
 */
#define COMMANDS_COUNT 3
static const struct api_command g_commands[COMMANDS_COUNT] = {
   { "sys", trepl_sys },
#ifdef USE_NET
   { "net", trepl_net },
#endif /* USE_NET */
   { "dsk", trepl_disk }
};

TASK_RETVAL repl_command( const struct astring* cli ) {
   uint8_t i = 0;

   for( i = 0 ; COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp_c( g_commands[i].command, CMD_MAX_LEN, cli, ' ' ) ) {
         return g_commands[i].callback( cli );
      }
   }
   
   return RETVAL_NOT_FOUND;
}

TASK_RETVAL trepl_task() {
   char c = '\0';
   const struct astring* line;
   //struct CHIIPY_TOKEN* token;
   //struct astring* arg;
   uint8_t i = 0;
   uint8_t retval = 0;

   adhd_task_setup();

   if( !(g_console_flags & CONSOLE_FLAG_INITIALIZED) ) {
      for( i = 0 ; 8 > i ; i++ ) {
         tprintf( qd_logo[i] );
         tprintf( CONSOLE_NEWLINE );
      }
      tprintf( "EL console v" VERSION CONSOLE_NEWLINE );
      tprintf( "ptr %d bytes" CONSOLE_NEWLINE, sizeof( void* ) );
      tprintf( "ready" CONSOLE_NEWLINE );
      g_console_flags |= CONSOLE_FLAG_INITIALIZED;
   }

   if( !twaitc() ) {
      adhd_yield();
   }

   c = tgetc();
   /* Dynamically allocate the line buffer so we can clear it from memory
    * during other programs. Add +1 so there's always a NULL.
    */
   line = alpha_astring(
      adhd_get_pid(), REPL_MID_LINE, REPL_LINE_SIZE_MAX + 1, NULL );

   if(
      line->len + 1 >= line->sz ||
      (('\r' == c || '\n' == c) && 0 == line->len)
   ) {
      /* Line would be too long if we accepted this char. */
      tprintf( CONSOLE_NEWLINE );
      tprintf( "invalid" CONSOLE_NEWLINE );
      alpha_astring_clear( adhd_get_pid(), REPL_MID_LINE );
      adhd_yield();
      adhd_continue_loop();
   }

   switch( c ) {
      case '\r':
      case '\n':
         tprintf( CONSOLE_NEWLINE );
         retval = repl_command( line );

         if( RETVAL_NOT_FOUND == retval ) {
            tprintf( "invalid" CONSOLE_NEWLINE );
         } else if( RETVAL_BAD_ARGS == retval ) {
            tprintf( "bad arguments" CONSOLE_NEWLINE );
         } else {
            tprintf( "ready" CONSOLE_NEWLINE );
         }
         alpha_astring_clear( adhd_get_pid(), REPL_MID_LINE );
         break;

      default:
         //chiipy_lex_tok( c, token );
         alpha_astring_append( adhd_get_pid(), REPL_MID_LINE, c );
         tputc( c );
         break;
   }

   adhd_yield();
   adhd_end_loop();
}
