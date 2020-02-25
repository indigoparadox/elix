
#include "../code16.h"

#include "repl.h"
#include <mfat.h>
#include <mbmp.h>
#include "../net/net.h"

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

/* Utility Functions */

CONSOLE_CMD g_repl_line_handler = NULL;

void repl_set_line_handler( CONSOLE_CMD new_handler ) {
   tprintf( "setting\n" );
   g_repl_line_handler = new_handler;
}

const char* trepl_tok( struct astring* cli, uint8_t idx ) {
   const char* tok = NULL;
   STRLEN_T sigil_start = 0;
   STRLEN_T i = 0;
   STRLEN_T len = 0;
   STRLEN_T sigil_val = 0;
   STRLEN_T sigil_len = 0;

   tok = alpha_tok( cli, '\0', idx );
   if( NULL == tok ) {
      return tok;
   }
   len = alpha_strlen_c( tok, REPL_LINE_SIZE_MAX );
#ifdef DEBUG_REPL
   tprintf( "tok: %s (%d)\n", tok, len );
#endif /* DEBUG_REPL */
   for( i = 0 ; len > i ; i++ ) {
      /* TODO: Deal with double sigil/replace var in buffer. */
      if( '$' == tok[i] && 0 >= sigil_start ) {
         /* Start grabbing the variable index. */
         sigil_start = i + 1;
         sigil_len = 0;

      } else if( alpha_isdigit( tok[i] ) && sigil_start > 0 ) {
         /* Continue grabbing the variable index. */
         sigil_len++;

         if( i + 1 >= len || !alpha_isdigit( tok[i + 1] ) ) {
            /* Stop grabbing the variable index. */
#ifdef DEBUG_REPL
            tprintf( "sigil capture ends at: %d\n", i );
            tprintf( "sigil start: %d len: %d\n", sigil_start, sigil_len );
#endif /* DEBUG_REPL */
            sigil_val = alpha_atou_c( &(tok[sigil_start]), sigil_len, 10 );
            sigil_start = 0;
#ifdef DEBUG_REPL
            tprintf( "getting cell #%d\n", sigil_val );
#endif /* DEBUG_REPL */
            tok = mget( adhd_get_pid_by_gid( "repl" ), sigil_val, len + 1 );
            if( NULL != tok ) {
               return tok;
            }

         }
      }
   }
   
   return tok;
}

/* = Command Callbacks */

#ifdef USE_NET

static TASK_RETVAL tnet_start( struct astring* cli ) {
   tprintf( "start?\n" );
   adhd_launch_task( net_respond_task );
   return RETVAL_OK;
}

static TASK_RETVAL tnet_rcvd( struct astring* cli ) {
   const int* received = NULL;
   TASK_PID pid;

   pid = adhd_get_pid_by_gid( "netp" );
   received = mget( pid, NET_MID_RECEIVED, sizeof( int ) );
   tprintf( "frames rcvd: %d\n", *received );

   return RETVAL_OK;
}

#define NET_COMMANDS_COUNT 2
const struct api_command g_net_commands[NET_COMMANDS_COUNT] = {
   { "rcvd", tnet_rcvd },
   { "start", tnet_start },
};

static TASK_RETVAL trepl_net( struct astring* cli ) {
   const char* tok;
   uint8_t i = 0;

   tok = trepl_tok( cli, 1 );
   if( NULL == tok ) {
      return 1;
   }

   for( i = 0 ; NET_COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp_cc(
         tok, CMD_MAX_LEN, g_net_commands[i].command, CMD_MAX_LEN, '\0',
         false, false
      ) ) {
         return g_net_commands[i].callback( cli );
      }
   }

   return RETVAL_BAD_ARGS;
}

#endif /* USE_NET */

static TASK_RETVAL tsys_proc( struct astring* cli ) {
   const char* task_gid = NULL;
   TASK_PID i = 0;

   for( i = 0 ; ADHD_TASKS_MAX > i ; i++ ) {
      task_gid = adhd_get_gid_by_pid( i );
      if( NULL != task_gid ) {
         tprintf( "%d\t\"%s\"\n", i, task_gid );
      }
   }
   
   return RETVAL_OK;
}

static TASK_RETVAL tsys_exit( struct astring* cli ) {
   g_system_state = SYSTEM_SHUTDOWN;
   return RETVAL_OK;
}

static TASK_RETVAL tsys_mem( struct astring* cli ) {
   tprintf(
      "mem:\nused: %d\ntotal: %d\nfree: %d\n",
      get_mem_used(), MEM_HEAP_SIZE, MEM_HEAP_SIZE - get_mem_used()
   );
   return RETVAL_OK;
}

#ifdef USE_DISK

static TASK_RETVAL tdisk_dir( struct astring* cli ) {
   const char* tok;
   uint16_t offset = 0;
   char filename[13];
   uint8_t attrib = 0;
   char attrib_str[5];
   uint32_t size = 0;

   offset = mfat_get_root_dir_offset( 0, 0 );
   offset = mfat_get_dir_entry_first_offset( offset, 0, 0 );

   tok = trepl_tok( cli, 1 );

   mzero( filename, 13 );
   mzero( attrib_str, 5 );

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

      /* Print the entry. */
      tprintf( "- %s\t%12s\t%4d\t%6d\n", attrib_str, filename,
         mfat_get_dir_entry_cyear( offset, 0, 0 ) + 1980, size );

      /* Get the next entry. */
      offset = mfat_get_dir_entry_next_offset( offset, 0, 0 );
   } while( 0 < offset );

   return RETVAL_OK;
}

#if 0
static TASK_RETVAL tdisk_fat( struct astring* cli ) {
   uint16_t i = 0;
   uint16_t entries_count = 0;
   uint16_t fat_entry = 0;
   const char* tok;
   uint8_t display_dec = 0;

   i = 2;
   tok = trepl_tok( cli, i );
   while( NULL != tok ) {
      /* Determine display mode. */
      if( !alpha_cmp_cc( "d", 1, tok, 1, ' ', false, false ) ) {
         display_dec = 1;
      } else if( 0 < alpha_atou_c( tok, 5 /* TODO */, 10 ) ) {
         if( display_dec ) {
            /* Interpret as decimal. */
            entries_count = alpha_atou_c( tok, 5, 10 );
            fat_entry = mfat_get_fat_entry( entries_count, 0, 0, 0 );
            tprintf( "fat entry %5d: %5d\n", entries_count, fat_entry );
            return RETVAL_OK;
         } else {
            /* Interpret as hex. */
            entries_count = alpha_atou_c( tok, 5, 16 );
            fat_entry = mfat_get_fat_entry( entries_count, 0, 0, 0 );
            tprintf( "fat entry %4x: %4x\n", entries_count, fat_entry );
            return RETVAL_OK;
         }
      }
      i++;
      tok = trepl_tok( cli, i );
   }

   entries_count = mfat_get_entries_count( 0, 0 );

   tprintf( "\n00000\t" );
   for( i = 0 ; entries_count > i ; i++ ) {
      fat_entry = mfat_get_fat_entry( i, 0, 0, 0 );

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
#endif

#ifndef USE_DISK_RO

static TASK_RETVAL tdisk_touch( struct astring* cli ) {
   const char* tok;
   uint32_t offset = 0;

   tok = trepl_tok( cli, 1 );
   if( NULL == tok ) {
      return RETVAL_BAD_ARGS;
   }

   /* See if the file exists already. */
   offset = mfat_get_root_dir_offset( 0, 0 );
   offset = mfat_get_dir_entry_offset( tok, MFAT_FILENAME_LEN, offset, 0, 0 );
   if( 0 < offset ) {
      /* TODO: Update file date. */
      tprintf( "file exists\n" );
      return RETVAL_BAD_ARGS;
   }

   /* Create the file. */
   offset = mfat_get_root_dir_offset( 0, 0 );
   offset = mfat_get_dir_entry_free_offset(
      offset, mfat_get_root_dir_entries_count( 0, 0 ) * 32, 0, 0 );
   if( 0 >= offset ) {
      tprintf( "directory full\n" );
      return RETVAL_BAD_ARGS;
   }

   mfat_set_dir_entry_name( tok, offset, 0, 0 );
   mfat_set_dir_entry_size( 0, offset, 0, 0 );
   tprintf( "create\n" );

   return RETVAL_OK;
}

#endif /* !USE_DISK_RO */

static TASK_RETVAL tdisk_cat( struct astring* cli ) {
   const char* tok;
   uint32_t offset = 0;
   char buffer[MFAT_FILENAME_LEN + 1];
   uint32_t written = 0;
   uint32_t file_size = 0;

   mzero( buffer, MFAT_FILENAME_LEN );

   tok = trepl_tok( cli, 1 );
   if( NULL == tok ) {
      return RETVAL_BAD_ARGS;
   }

   offset = mfat_get_root_dir_offset( 0, 0 );
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

#if 0
#ifdef USE_BMP

static TASK_RETVAL tdisk_bmp( struct astring* cli ) {
   const char* tok;
   FILEPTR_T offset = 0;

   tok = trepl_tok( cli, 1 );
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

#endif /* USE_BMP */
#endif

#endif /* USE_DISK */

TASK_RETVAL trepl_let( struct astring* cli ) {
   uint8_t idx = 0;
   STRLEN_T len = 0;
   const char* tok = NULL;
   char* memstr = NULL;
   STRLEN_T i = 0;

   /* Get memory index. */
   tok = trepl_tok( cli, 1 );
   if( NULL == tok ) {
      return RETVAL_BAD_ARGS;
   }
   idx = alpha_atou_c( tok, CMD_MAX_LEN, 10 );

   /* Get and allocate value. */
   tok = trepl_tok( cli, 2 );
   if( NULL == tok ) {
      return RETVAL_BAD_ARGS;
   }
   len = alpha_strlen_c( tok, CMD_MAX_LEN ) + 1;
   memstr = mget( adhd_get_pid_by_gid( "repl" ), idx, len );

   /* Copy value. */
   for( i = 0 ; len > i ; i++ ) {
      memstr[i] = tok[i];
   }
   memstr[i] = '\0';

#ifdef DEBUG_REPL
   tprintf( "set $%d to %s\n", idx, memstr );
#endif /* DEBUG_REPL */

   return RETVAL_OK;
}

TASK_RETVAL trepl_print( struct astring* cli ) {
   const char* tok = NULL;

   tok = trepl_tok( cli, 1 );
   if( NULL == tok ) {
      return RETVAL_BAD_ARGS;
   }

   tprintf( "%s\n", tok );  

   return RETVAL_OK;
}

TASK_RETVAL trepl_if( struct astring* cli ) {
   const char* tok1 = NULL;
   const char* tok2 = NULL;
   STRLEN_T i = 0;
   STRLEN_T len1 = 0;
   STRLEN_T len2 = 0;

   tok1 = trepl_tok( cli, 1 );
   tok2 = trepl_tok( cli, 2 );
   len1 = alpha_strlen_c( tok1, REPL_LINE_SIZE_MAX );
   len2 = alpha_strlen_c( tok2, REPL_LINE_SIZE_MAX );

   /* Verify same length. */
   if( len1 != len2 ) {
#if DEBUG_REPL
      tprintf( "false\n" );
#endif /* DEBUG_REPL */
      return RETVAL_FALSE;
   }

   /* Verify same value. */
   for( i = 0 ; len1 > i && len2 > i ; i++ ) {
      if( tok1[i] != tok2[i] ) {
#if DEBUG_REPL
         tprintf( "false\n" );
#endif /* DEBUG_REPL */
         return RETVAL_FALSE;
      }
   }

   /* Verification passed. Do command. */

   alpha_replace( '\0', ' ', cli );
   alpha_astring_rtrunc( cli, (len1 + len2 + 3 /* Spaces */ + 3 /* ife */) );

   repl_command( cli );

#if DEBUG_REPL
   tprintf( "true\n" );
#endif /* DEBUG_REPL */

   return RETVAL_TRUE;
}

/**
 * Map typed commands to callbacks.
 */
#define COMMANDS_COUNT 10
static const struct api_command g_commands[COMMANDS_COUNT] = {
#ifdef USE_DISK
   { "cat", tdisk_cat },
   { "dir", tdisk_dir },
#endif /* USE_DISK */
   { "exit", tsys_exit },
   /* { "for", trepl_for }, */
   { "ife", trepl_if },
   { "let", trepl_let },
   { "mem", tsys_mem },
#ifdef USE_NET
   { "net", trepl_net },
#endif /* USE_NET */
   { "print", trepl_print },
   { "proc", tsys_proc }
#ifdef USE_DISK
#ifndef USE_DISK_RO
   , { "touch", tdisk_touch }
#endif /* !USE_DISK_RO */
#endif /* USE_DISK */
};

TASK_RETVAL repl_command( struct astring* cli ) {
   uint8_t i = 0;
   TASK_RETVAL retval = 0;

   alpha_replace( ' ', '\0', cli );

   for( i = 0 ; COMMANDS_COUNT > i ; i++ ) {
      if(
         0 == alpha_cmp_c( g_commands[i].command, CMD_MAX_LEN, cli, '\0',
            false, 3 )
      ) {
         retval = g_commands[i].callback( cli );
         if( RETVAL_OK == retval ) {
            tprintf( "ready\n" );
            return retval;
         }
      }
   }

   /* Could not find an internal command. Try installed apps. */
   for( i = 0 ; g_console_apps_top > i ; i++ ) {
      retval = g_console_apps[i]( cli );
      if( RETVAL_NOT_FOUND != retval ) {
         return retval;
      }
   }

   return RETVAL_NOT_FOUND;
}

TASK_RETVAL trepl_task() {
   char c = '\0';
   struct astring* line;
   uint8_t i = 0;
   uint8_t retval = 0;
   uint8_t* flags = NULL;

   adhd_task_setup();

   adhd_set_gid( "repl" );

   flags = mget( adhd_get_pid(), REPL_MID_FLAGS, 1 );

   if( NULL == g_repl_line_handler ) {
      g_repl_line_handler = repl_command;
   }

   /* If we're not the current app using the console, uninitialize so that
    * we will reinitialize once whatever other app is done with it.
    */
   if( g_console_pid != 0 && g_console_pid != adhd_get_pid() ) {
      *flags &= ~REPL_FLAG_INITIALIZED;
      mfree( adhd_get_pid(), REPL_MID_LINE );
      adhd_yield();
   }

   if( !(*flags & REPL_FLAG_INITIALIZED) ) {
      for( i = 0 ; 8 > i ; i++ ) {
         tprintf( qd_logo[i] );
         tprintf( "\n" );
      }
      tprintf( "ELix console v" VERSION "\n" );
#ifndef REPL_NO_PRINTF_PTR
      tprintf( "ptr %d bytes\n", sizeof( void* ) );
#endif /* REPL_NO_PRINTF_PTR */
      tprintf( "ready\n" );
      *flags |= REPL_FLAG_INITIALIZED;
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
   assert( NULL != line );

   if(
      line->len + 1 >= (line->mem.sz - sizeof( struct astring )) ||
      (('\r' == c || '\n' == c) && 0 == line->len)
   ) {
      if( line->len + 1 >= (line->mem.sz - sizeof( struct astring )) ) {
         /* Line would be too long if we accepted this char. */
         tprintf( "\n" );
         tprintf( "linebuf\n" );
      }

      /* Reset line. */
      alpha_astring_clear( line );
      adhd_yield();
      adhd_continue_loop();
   }

   switch( c ) {
      case '\r':
      case '\n':
         /* Reset any pending ANSI flag. */
         *flags &= ~REPL_FLAG_ANSI_SEQ;

         tprintf( "\n" );
         retval = g_repl_line_handler( line );

         if( RETVAL_NOT_FOUND == retval ) {
            tprintf( "invalid\n" );
         } else if( RETVAL_BAD_ARGS == retval ) {
            tprintf( "bad arguments\n" );
         }
         alpha_astring_clear( line );
         break;

      case 127:
         /* Backspace */
         alpha_astring_trunc( line, 1 );
         tputc( c );
         break;

      case 27:
         *flags |= REPL_FLAG_ANSI_SEQ;
         break;

      default:
         if( *flags & REPL_FLAG_ANSI_SEQ ) {
            if( 'A' == c ) {
               /* Up */
            } else if( 'B' == c ) {
               /* Down */
            } else if( 'C' == c ) {
               /* Right */
            } else if( 'D' == c ) {
               /* Left */
            } else if( '[' == c ) {
               break;
            } else {
               tprintf( "\nansi+%d\n", c );
            }

            /* Reset pending ANSI flag. */
            *flags &= ~REPL_FLAG_ANSI_SEQ;
            break;
         }

         alpha_astring_append( line, c );
         tputc( c );
         break;
   }

   adhd_yield();
   adhd_end_loop();
}

