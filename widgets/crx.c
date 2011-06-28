/*
 * name: crx
 * description: Regular Expression Engine (light weight version) for C Language, using double-recursion and function pointers.
 * author: ken (hexabox) seto
 * date: 2009.08~09
 * license: BSD, GPL
 *
 * version: 0.13.13
 */
#include "crx.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TYPE_CHAR = 1,
    TYPE_CMD  = 2,
    TYPE_PREFIX = 4,
    TYPE_SUFFIX = 8,
    TYPE_OPEN = 16,
    TYPE_CLOSE = 32
} CMD_TYPE;

/*!
  \brief Regex CMD structure
  */
typedef struct {
    char id;		/*!< ID */
    int span;		/*!< Span */
    void* fcn;		/*!< function pointer */
    CMD_TYPE type;	/*!< Command type enum */
} Cmd;

/* Prototypes */
int match(char*, char*, char* );
/* function pointers */
#define CMD2    (int(*)(char*,char*))
#define CMD3    (int(*)(char*,char*,char*))

/* function shortcuts */
#define _CMD2(str)  int c_##str(char* pat, char* sam)
#define _CMD3(str)  int c_##str(char* pat, char* sam, char* endp)

/*
 * rules of commands:
 * 1. return number of consumed characters in sample string
 * 2. TYPE_CLOSE follows TYPE_OPEN immediately in command table
 */
inline _CMD2(achar);
inline _CMD2(any);
inline _CMD2(escape);
_CMD2(group);
_CMD3(multi);
_CMD2(option);

Cmd cmd_tbl[] = {
	{ '(',    0,    c_group,    TYPE_OPEN,			},
	{ ')',    1,    NULL,       TYPE_CLOSE,			},
	{ '[',    0,    c_option,   TYPE_OPEN,			},
	{ ']',    1,    NULL,       TYPE_CLOSE,			},
	{ '{',    0,    c_multi,    TYPE_SUFFIX|TYPE_OPEN,	},
	{ '}',    1,    NULL,       TYPE_CLOSE,			},
	{ '*',    1,    c_multi,    TYPE_SUFFIX,		},
	{ '+',    1,    c_multi,    TYPE_SUFFIX,		},
/*	{ '?',    1,    c_multi,    TYPE_SUFFIX,		},*/
	{ '.',    1,    c_any,      TYPE_CMD,			},
	{ '\\',    2,    c_escape,   TYPE_PREFIX,		},
	{ 0,    1,    c_achar,    TYPE_CHAR,			}
};

Cmd* get_cmd(char id)
{
    Cmd* cmd = &cmd_tbl[0];
    while (cmd->id != 0)
    {
        if (id == cmd->id)
            break;
        cmd++;
    }
    return cmd;
}

inline bool is_suffix(char id)
{
    Cmd* cmd = get_cmd(id);
    return (cmd ? (cmd->type & TYPE_SUFFIX) == TYPE_SUFFIX : false);
}

char* find_close(char* init, char stop)
{
    int cnt = 0;
    char* ptr = init;

    while (*ptr)
    {
        if (*ptr == *init)
            cnt++;
        else if (*ptr == stop)
            cnt--;

        if (cnt == 0)
            return ptr;

        ptr++;
    }
    return NULL;
}

char* get_next_pat(char* cur)   /* find next unit of pattern */
{
    Cmd* cmd = get_cmd(*cur);

    if (cmd->type & TYPE_OPEN)
    {
        cur = find_close(cur, (cmd+1)->id);
        if (cur)
            return (cur + 1);
    }
    return (cur + cmd->span);
}

/* -------------- command handlers ---------------- */

inline _CMD2(any) {return 1;}
inline _CMD2(achar) {return (*pat == *sam);}


_CMD2(group)    /* sub pattern */
{
    char *close = find_close(pat, ')');
    if (!close) return false;
    return match(pat+1, sam, close);
}


_CMD2(escape)
{
    char magic[16] = "";
    
    switch (*++pat)
    {
        case 'd':   /* digit */
            strcpy(magic, "[0-9]");
            break;
        case 'D':   /* non-digit */
            strcpy(magic, "[^0-9]");
            break;
        case 'x':   /* hex digit */
            strcpy(magic, "[0-9A-Fa-f]");
            break;
        case 'X':
            strcpy(magic, "[^0-9A-Fa-f]");
            break;
        case 'o':   /* octal digit */
            strcpy(magic, "[0-7]");
            break;
        case 'O':
            strcpy(magic, "[^0-7]");
            break;
        case 'w':   /* word character */
            strcpy(magic, "[0-9A-Za-z_]");
            break;
        case 'W':
            strcpy(magic, "[^0-9A-Za-z_]");
            break;
        case 'h':   /* head of word character */
            strcpy(magic, "[0-9A-Za-z]");
            break;
        case 'H':
            strcpy(magic, "[^0-9A-Za-z]");
            break;
        case 'a':   /* alphabetic character */
            strcpy(magic, "[A-Za-z]");
            break;
        case 'A':
            strcpy(magic, "[^A-Za-z]");
            break;
        case 'l':   /* lowercase character */
            strcpy(magic, "[a-z]");
            break;
        case 'L':
            strcpy(magic, "[^a-z]");
            break;
        case 'u':   /* uppercase character */
            strcpy(magic, "[A-Z]");
            break;
        case 'U':
            strcpy(magic, "[^A-Z]");
            break;
    }

    if (*magic)
        return match(magic, sam, strchr(magic, 0));
    else
        return (*pat == *sam);
}


_CMD2(option)
{
    bool not;
    char* from = NULL;
    char* to = NULL;
    char *close;

    close = pat;
    do {
        close = find_close(close, ']');
        if (!close) return false;
    } while (close[-1] == '\\');

    not = (pat[1] == '^');
    pat += not ? 2 : 1;

    while (pat < close)
    {
        if (*pat == '-' && from)
        {
            to = pat + 1;
            if (*to == '\\') to ++;
            if (to >= close) break;
            if (*sam >= *from && *sam <= *to)
                return (not ? 0 : 1);
            pat = to + 1;
            continue;
        }

        from = pat;
        if (*from == '\\') from ++;
        if (from >= close) break;
        if (*sam == *from)
            return (not ? 0 : 1);

        pat++;
    }

    return (not ? 1 : 0);
}

_CMD3(multi)
{
    /* repeated variables */
    Cmd* cmd = get_cmd(*pat);
    int  found;
    char* start_sam = sam;

    /* new local variables */
    int repeat = 0;
    int good_follows = 0;

    char *ends = strchr(sam, 0);
    char *good_sam = NULL;

    char *multi = get_next_pat(pat);
    char *next_pat = get_next_pat(multi);

    int min = 0, max = 0;

    switch (*multi)
    {
        case '{':  /* for range of repetition */
            {
                char* comma = strchr(multi, ',');
                char* close = strchr(multi, '}');

                min = atoi(multi + 1);
                if (comma && comma < close)
                    max = atoi(comma + 1);
                else
                    max = min;
            } break;
        case '+':
            min = 1;
            break;
        case '?':
            max = 1;
            break;
    }

    while (sam < ends)
    {
        found = (CMD2 cmd->fcn)(pat, sam);

        if (!found) break;  /* can be less than min */

        /* condition 1 */
        repeat ++;
        sam += found;   /* advance ptr */

        if (min && repeat < min)
            continue;

        if (*next_pat)
        {
            if (sam >= ends)
                break;

            found = match(next_pat, sam, endp);
            if (found)    /* condition 2 */
            {
                good_follows = found;
                good_sam = sam;
            }
        }

        if (max && repeat >= max)
            break;
    }

    /* return here */
    if (repeat < min)
        found = 0;

    else if (!*next_pat)
        found = sam - start_sam;

    else if (good_sam)  /* *next_pat > 0 */
        found = good_sam + good_follows - start_sam;

    else if (!min)
    {
        if (repeat == 0)    /* no match before *multi */
            found = match(next_pat, sam, endp);

        else if (!good_sam) /* repeat > 0 is wrongly match before *multi */
            found = match(next_pat, start_sam, endp);
    }

    else found = 0;

    return found;
}


/* -------------- core functions ---------------- */

G_MODULE_EXPORT char* regex(char* pat, char* sam, int* len)
{
    *len = 0;
    while (*pat && *sam)
    {
        *len = match(pat, sam, strchr(pat, '\0'));
        if (*len > 0)
            break;
        sam++;
    }

    if (*len > 0)
        return sam;
    else
        return NULL;
}

/*
 * return: # found in sam
 */
int match(char* pat, char* sam, char* endp)
{
    Cmd* cmd;
    char* next_pat;
    int  found;
    char* start_sam = sam;


    while (pat < endp)
    {
        if (*pat == 0)  break;
        if (*sam == 0)  return 0;

        next_pat  = get_next_pat(pat);
        cmd = get_cmd(*pat);

        if (next_pat  &&  pat < endp  &&  is_suffix(*next_pat))
        {
            cmd = get_cmd(*next_pat);
            return (sam - start_sam) + (CMD3 cmd->fcn)(pat, sam, endp);
        }
        else
        {
            found = (CMD2 cmd->fcn)(pat, sam);
            if (!found) return 0;

            sam += found;
            pat = next_pat;
        }
    }

    return (sam - start_sam);
}
