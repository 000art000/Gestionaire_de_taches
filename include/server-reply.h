#ifndef SERVER_REPLY_H
#define SERVER_REPLY_H

#define SERVER_REPLY_OK 0x4f4b     // 'OK'
#define SERVER_REPLY_ERROR 0x4552  // 'ER'

#define SERVER_REPLY_ERROR_NOT_FOUND 0x4e46  // 'NF'
#define SERVER_REPLY_ERROR_NEVER_RUN 0x4e52  // 'NR'

#define SATURND_REQUEST_PIPE "D_CMD/saturnd-request-pipe"
#define SATURND_REPLY_PIPE "D_CMD/saturnd-reply-pipe"

#define FILE_CMD "D_CMD/cmds"
#define DIRECTORY_FILES_CMD "D_CMD"
#define OUTPUT "output"
#define ERROR "erreur"
#define T_E "times and exitcodes"


#endif // SERVER_REPLY_H
