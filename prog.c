#include "shell_body.h"

void    free_job(job work)
{
    int     i, j;

    i = 0;
    while (i < work.count_prog)
    {
        j = 0;
        while (j < work.prog[i]->count_arg)
        {
            free (work.prog[i]->arg[j]);
            j++;
        }
        if (work.prog[i]->count_arg)
            free (work.prog[i]->arg);
        if (work.prog[i]->input_file)
            free (work.prog[i]->input_file);
        if (work.prog[i]->output_file)
            free (work.prog[i]->output_file);
        free (work.prog[i]);
        i++;
    }
    free (work.prog);   
}

void    shell_exit(job work, program* prog)
{
    //printf ("\e[2J\e[3J\e[r");
    if (prog->count_arg == 0)
    {
        if (hist->head != NULL)
        {
            delete_history();
        }
        free_job(work);
    }
    free (input_str); 
    exit(0);
}

void    shell_cd(program* prog)
{
    char    path[256];

    if (prog->count_arg == 1)
    {
        //printf ("Old: %s\n", getenv("PWD"));
        if (!chdir (prog->arg[0]))
        {
            getcwd (path, 256);
            setenv ("PWD", path, 1);
            //printf ("New: %s\n", getenv("PWD"));
        }
        else
            printf ("No such file or directory!\n");
    }
    else
        printf ("Strange arguments!\n");
}

void    shell_pwd(program* prog)
{
    //printf ("\e[38;5;39m%s\e[0m\n", getenv ("PWD"));
    if (prog->count_arg == 0)
        printf ("%s\n", getenv ("PWD"));
    else
        printf ("Strange arguments!\n");
}

void    delete_history()
{
    node*   phist;
    node*   nexthist;

    phist = hist->head;
    while (phist != NULL)
    {
        nexthist = phist->next;
        free (phist->str);
        free (phist);
        phist = nexthist;
    }
    hist->head = NULL;
    hist->tail = NULL;
}

void    shell_history(program* prog)
{
    node*   phist;

    phist = hist->head;
    if (prog->count_arg == 0)
    {
        while (phist != NULL)
        {
            printf ("%d %s\n", phist->num, phist->str);
            phist = phist->next;
        }
    }
    else
        printf ("Strange arguments!\n");
}

void    shell_put_history(char* str)
{
    if (hist->head == NULL)
    {
        hist->head = (node*) malloc (sizeof (node));
        hist->head->str = (char*) malloc ((strlen (str) + 1) * sizeof (char));
        hist->head->num = 1;
        strcpy (hist->head->str, str);
        hist->head->next = NULL;
        hist->head->prev = NULL;
        hist->tail = hist->head;
        hist->prevhist = hist->head;
    }
    else
    {
        if ((hist->prevhist->num + 1) > hist->max_num)
        {
            free (hist->head->str);
            hist->head = hist->head->next;
            free (hist->head->prev);
        }
        hist->tail = (node*) malloc (sizeof (node));
        hist->tail->str = (char*) malloc ((strlen (str) + 1) * sizeof (char));
        hist->tail->num = hist->prevhist->num + 1;
        strcpy (hist->tail->str, str);
        hist->tail->next = NULL;
        hist->tail->prev = hist->prevhist;
        hist->prevhist->next = hist->tail;
        hist->prevhist = hist->tail;
    }
}

void    special_value(char* shell_path)
{
    struct passwd*  user;
    pid_t           id;
    char            id_str[20], shell_pid[20];

    id = getuid ();
    user = getpwuid (id);
    if (user != NULL)
    {
        setenv ("USER", user->pw_name, 1);
        setenv ("HOME", user->pw_dir, 1);
        setenv ("SHELL", shell_path, 1);
        sprintf (id_str, "%d", user->pw_uid);
        setenv ("UID", id_str, 1);
        setenv ("PWD", user->pw_dir, 1);
        sprintf (shell_pid, "%d", getpid ());
        setenv ("PID", shell_pid, 1);
    }
}

char*   command_input(char* str)
{
    int     i;
    char    symbol;
    i = 1;
    //printf ("\e[38;5;39m");
    while ((symbol = getchar()) != EOF)
    {
        if (symbol != '\n')
        {
            if (i == 1)
                str = (char*) malloc (sizeof (char));
            else
                str = (char*) realloc (str, i * sizeof (char));
            str[i-1] = symbol;
            i++;
        }
        else
            break;
    }
    if (symbol == EOF)
        printf ("\n");
    //printf ("\e[0m");
    if (i != 1)
    {
        str = (char*) realloc (str, i * sizeof (char));
        str[i-1] = '\0';
        return (str);
    }
    else
        return (NULL);
}

int     quote_str_slasher(char* str, int i, char symbol, int str_size)
{
    int j, slash_count, res;

    j = i;
    slash_count = 1;
    //str_size = strlen (str);
    while (str[j] == '\\')
    {
        j++;
        slash_count++;
    }
    //printf ("slashcount: %d\n", slash_count);
    if (!(slash_count % 2))
    {
        res = i + ((slash_count / 2));
    }
    else
    {
        res = i + ((slash_count - 1) / 2);
    }
    //printf ("str_size = %d j = %d str: %s\n", str_size, j, str);
    if (str[j] == symbol && j < str_size)
    {
        j = i + 1;
       // slash_count--;
        while (slash_count)
        {
            //printf ("yes\n");
            if (str[j-1] == str[j] && str[j] == '\\')
            {
                memmove (&str[j-1], &str[j], (str_size - j) + 1);
                str_size--;
            }
            else
            {
                break;
            }
            j++;
            i++;
            slash_count--;
        }
    }
    return (res + 1);
}

int     str_slasher(char* str, int i, char symbol, int str_size)
{
    int j, slash_count, res;

    j = i;
    slash_count = 1;
    //str_size = strlen (str);
    while (str[j] == '\\')
    {
        j++;
        slash_count++;
    }
    //printf ("slashcount: %d\n", slash_count);
    if (!(slash_count % 2))
    {
        res = i + ((slash_count / 2) - 1);
    }
    else
    {
        res = i + ((slash_count - 1) / 2);
        if (str[j] == symbol && (symbol == ';' || symbol == '|'))
            str[j] = '\0';
    }
    //printf ("str_size = %d j = %d str: %s\n", str_size, j, str);
    if ((str[j] == symbol || (str[j] == '\0' && (symbol == ';' || symbol == '|'))) && j < str_size)
    {
        j = i + 1;
       // slash_count--;
        while (slash_count)
        {
            //printf ("yes\n");
            if (str[j-1] == str[j] && str[j] == '\\')
            {
                memmove (&str[j-1], &str[j], (str_size - j) + 1);
                str_size--;
            }
            else
            {
                if (str[j-1] != str[j] && str[j-1] == '\\')
                {
                    if (str[j] == symbol)
                    {
                        str[j] = symbol;
                        memmove (&str[j-1], &str[j], (str_size - j) + 1);
                        str_size--;
                        slash_count--;
                    }
                }
                break;
            }
            j++;
            i++;
            slash_count--;
        }
    }
    return (res + 1);
}

char*   delete_comments(char* str)
{
    char*   find_comment;
    int     i, solo_quote_indicator, quote_indicator, str_size;

    i = 0;
    solo_quote_indicator = quote_indicator = FALSE;
    str_size = strlen (str);
    if ((find_comment = strchr (str, '#')) != NULL)
    {
        if (find_comment == &str[0])
        {
            free (str);
            str = NULL;
        }
        else
        {
            while (str[i] != '\0')
            {
                if (str[i] == '\'' && quote_indicator == FALSE)
                {
                    if (solo_quote_indicator == TRUE)
                        solo_quote_indicator = FALSE;
                    else
                        solo_quote_indicator = TRUE;
                }
                if (str[i] == '"' && solo_quote_indicator == FALSE)
                {
                    if (quote_indicator == TRUE)
                        quote_indicator = FALSE;
                    else
                        quote_indicator = TRUE;
                }
                //Начало магии со слешем
                if (str[i] == '\\' && solo_quote_indicator == FALSE && quote_indicator == FALSE)
                {
                    //printf ("do str: %s\n", &str[i]);
                    i = str_slasher(str, i, '#', str_size);   
                    // ("posle str: %s\n", &str[i]);
                }
                //Конец магии со слешем
                if (str[i] == '#' && solo_quote_indicator == FALSE && quote_indicator == FALSE && str[i-1] == ' ')
                {
                    str = (char*) realloc (str, ((i + 1) * sizeof (char)));
                    str[i] = '\0';
                    break;
                }
                i++;
            }
        }
    }
    return (str);
}

char*   history_str(char* str)
{
    int     i, solo_quote_indicator, quote_indicator, number, new_str_part_size, slash_count, end_size, print_indicator;
    char*   new_str_part;
    char*   end;
    node*   phist;

    print_indicator = FALSE;
    if (strchr (str, '!'))
    {
        i = 0;
        solo_quote_indicator = quote_indicator = FALSE;
        while (str[i] != '\0')
        {
            if (str[i] == '\'' && quote_indicator == FALSE)
            {
                if (solo_quote_indicator == TRUE)
                    solo_quote_indicator = FALSE;
                else
                    solo_quote_indicator = TRUE;
            }
            if (str[i] == '"' && solo_quote_indicator == FALSE)
            {
                if (quote_indicator == TRUE)
                    quote_indicator = FALSE;
                else
                    quote_indicator = TRUE;
            }
            if (str[i] == '!' && solo_quote_indicator == FALSE)
            {
                slash_count = 0;
                if (i != 0 && str[i-1] == '\\')
                {
                    slash_count = 1;
                    while (str[i - slash_count] == '\\')
                    {
                        slash_count++;
                        if ((i - slash_count) < 0)
                            break;
                    }
                    slash_count--;
                    if (quote_indicator == FALSE)
                    {
                        i = str_slasher(str, i-slash_count, '!', strlen (str));
                        i--;
                    }
                    else
                        i = quote_str_slasher(str, i-slash_count, '!', strlen (str));
                }
                if (str[i+1] != '\0' && slash_count % 2 == 0)
                {
                    number = strtol (&str[i+1], &end, 10);
                    if (end != NULL && slash_count == 0)
                    {
                        if (hist->head != NULL && hist->tail->num >= number && number > 0)
                        {
                            phist = hist->head;
                            while (phist->num != number)
                                phist = phist->next;
                            end_size = strlen (end);
                            new_str_part_size = strlen (phist->str) + end_size;
                            new_str_part = (char*) malloc ((new_str_part_size + 1) * sizeof (char));
                            strcpy (new_str_part, phist->str);
                            strcat (new_str_part, end);
                            str[i] = '\0';
                            str = (char*) realloc (str, (strlen (str) + new_str_part_size + 1) * sizeof (char));
                            strcat (str, new_str_part);
                            if (end_size != 0)
                                print_indicator = TRUE;
                            free (new_str_part);
                        }
                        else
                            printf ("History number not found!\n");
                    }
                }
            }
            i++;
        }
        if (print_indicator == TRUE)
            printf ("%s\n", str);
    }
    return (str);
}

void    command_line()
{
    char*   str;

    str = NULL;
    while (!str)
    {
        while (!str)
        {
            printf ("\e[48;5;176m%s~₽:\e[0m ", getenv("USER"));
            //printf ("%s ", getenv("USER"));
            str = command_input(str);
        }
        str = history_str(str);
        shell_put_history(str);
        str = delete_comments(str);
        //str = history_str(str);
    }
    input_str = str;
    job_controller(str);
}

int     str_transfer(char* str, int str_size, int transfer)
{
    int i;

    i = 0;
    while (transfer <= str_size)
    {
        if (str[transfer] != '\0')
            break;
        else
            transfer++;
        i++;
    }
    if (transfer > str_size)
        return 0;
    else
        return (i);
}

int     command_another(program* prog)
{
    pid_t pid;
    int   status;

    if ((pid = fork ()) == 0)
    {
        if ((execvp (prog->arg[0], prog->arg)) == -1)
        printf ("This command not found!");
            exit (0);
    }
    if (pid == -1)
        printf ("%s\n", strerror(errno));
    waitpid (pid, &status, 0);
    return 0;
}

char*   special_value_analyzer(char* str, int i, int str_size)
{
    char*   pfind_str;
    char*   pfind_char;
    char*   str_end;
    char*   sp_value;
    char*   sp_value_name;
    int     sp_value_size, str_end_size, indicator, new_str_size, number;
    char    num_in_str[17];
    char*   end;

    indicator = FALSE;
    if ((pfind_str = strstr (&str[i], "{USER}")) && pfind_str == &str[i + 1])
    {
            sp_value_name = "USER";
            indicator = TRUE;
    }
    else
    {
        if ((pfind_str = strstr (&str[i], "{HOME}")) && pfind_str == &str[i + 1])
        {
            sp_value_name = "HOME";
            indicator = TRUE;
        }
        else
        {
            if ((pfind_str = strstr (&str[i], "{SHELL}")) && pfind_str == &str[i + 1])
            {
                sp_value_name = "SHELL";
                indicator = TRUE;
            }
            else
            {
                if ((pfind_str = strstr (&str[i], "{UID}")) && pfind_str == &str[i + 1])
                {
                    sp_value_name = "UID";
                    indicator = TRUE;
                }
                else
                {
                    if ((pfind_str = strstr (&str[i], "{PWD}")) && pfind_str == &str[i + 1])
                    {
                        sp_value_name = "PWD";
                        indicator = TRUE;
                    }
                    else
                    {
                        if ((pfind_str = strstr (&str[i], "{PID}")) && pfind_str == &str[i + 1])
                        {
                                sp_value_name = "PID";
                            indicator = TRUE;
                        }
                        else
                        {
                            if ((pfind_str = strstr (&str[i], "#")) && pfind_str == &str[i + 1])
                            {
                                pfind_str[0] = '\0';
                                sprintf (num_in_str, "%d", argc_main);
                                str_end_size = strlen (&pfind_str[1]);
                                sp_value_size = strlen (num_in_str);
                                str_end = (char*) malloc ((str_end_size + sp_value_size + 1) * sizeof (char));
                                strcpy (str_end, num_in_str);
                                strcat (str_end, &pfind_str[1]);
                                new_str_size = (str_size + (strlen (str_end) - str_end_size) + 1) * sizeof (char);
                                str = (char*) realloc (str, new_str_size);
                                str[i] = '\0';
                                strcat (&str[i], str_end);
                                free (str_end);
                            }
                            else
                            {
                                number = strtol (&str[i+1], &end, 10);
                                if (end != NULL)
                                {
                                    if (number >= 0 && number < argc_main)
                                    {
                                        str_end_size = strlen (end);
                                        sp_value_size = strlen (argv_main[number]);
                                        str_end = (char*) malloc ((str_end_size + sp_value_size + 1) * sizeof (char));
                                        strcpy (str_end, argv_main[number]);
                                        strcat (str_end, end);
                                        new_str_size = (str_size + (strlen (str_end) - str_end_size) + 1) * sizeof (char);
                                        str = (char*) realloc (str, new_str_size);
                                        str[i] = '\0';
                                        strcat (&str[i], str_end);
                                        free (str_end);
                                    }
                                    else
                                        printf ("The argument is not found!\n");
                                }
                            }
                        }
                    }  
                }
            }  
        }       
    }  
    if (indicator == TRUE)
    {
        pfind_char = strchr (&str[i], '}');
        str_end_size = strlen (&pfind_char[1]);
        sp_value = getenv (sp_value_name);
        sp_value_size = strlen (sp_value);
        str_end = (char*) malloc ((str_end_size + sp_value_size + 1) * sizeof (char));
        strcpy (str_end, sp_value);
        strcat (str_end, &pfind_char[1]);
        str_end_size = strlen (str_end);
        new_str_size = (((str_size + 1) - strlen (&str[i+1])) + str_end_size) * sizeof (char);
        str = (char*) realloc (str, new_str_size);
        str[i] = '\0';
        strcat (&str[i], str_end);
        free (str_end);
        //echo "Hi ${SHELL}!"
    }
    return (str);
}

int    count_chr(char* str, char symbol)
{
    int     count;
    char*   find_str;

    count = 0;
    find_str = str;
    while ((find_str = strchr (find_str, symbol)))
    {
        find_str = &find_str[1];
        count++;
    }
    return (count);
}

void    job_controller(char* str)
{
    int     i, quote_indicator, solo_quote_indicator, prev_str_num, count, str_size, slash_count;

    i = prev_str_num = 0;
    count = 1;
    solo_quote_indicator = quote_indicator = FALSE;
    str_size = strlen (str);
    while (str[i] != '\0')
    {
        if (str[i] == '\'' && quote_indicator == FALSE)
        {
            if (solo_quote_indicator == TRUE)
                solo_quote_indicator = FALSE;
            else
                solo_quote_indicator = TRUE;
        }
        if (str[i] == '"' && solo_quote_indicator == FALSE)
        {
            if (quote_indicator == TRUE)
                quote_indicator = FALSE;
            else
                quote_indicator = TRUE;
        }
        //Подстановка переменных окружения
        if (str[i] == '$' && solo_quote_indicator == FALSE)
        {
            if (i > 0 && str[i-1] == '\\')
            {
                slash_count = 1;
                while (str[i-slash_count] == '\\')
                {
                    slash_count++;
                    if (i-slash_count < 0)
                        break;
                }
                slash_count--;
                i = str_slasher(str, i-slash_count, '$', str_size);
                i--;
                if ((slash_count % 2) == 0)
                {
                    str = special_value_analyzer(str, i, str_size);   
                }
                str_size += (strlen (&str[i]) - (str_size - (i + 1)));
            }
            else
            {
                str = special_value_analyzer(str, i, str_size);
                str_size += (strlen (&str[i]) - (str_size - (i + 1)));
            }
        }
        //Начало магии со слешем
        //printf ("i: %d", i);
        if (str[i] == ';' && solo_quote_indicator == FALSE)
        {
            if (i > 0 && str[i-1] == '\\')
            {
                slash_count = 1;
                while (str[i-slash_count] == '\\')
                {
                    slash_count++;
                    if (i-slash_count < 0)
                        break;
                }
                slash_count--;
                if (quote_indicator == FALSE)
                {
                    i = str_slasher(str, i-slash_count, ';', str_size);
                    i--;
                }
                else
                    i = quote_str_slasher(str, i-slash_count, ';', str_size);
                //i--;
                //printf ("i: %d str: %s\n",i, &str[prev_str_num]);
                if (!(slash_count % 2) && quote_indicator == FALSE)
                {
                    str[i] = '\0';
                    command_parser(&str[prev_str_num]);
                    prev_str_num = i + 1;
                    count++;
                }
                str_size += (strlen (&str[i]) - (str_size - (i + 1))); 
            }
            else
            {
                if (quote_indicator == FALSE)
                {
                    str[i] = '\0';
                    command_parser(&str[prev_str_num]);
                    prev_str_num = i + 1;
                    count++;
                }
            }
        }
        i++;
    }
    if (count >= 1)
    {
        //printf ("str: %s\n", &str[prev_str_num]);
        command_parser(&str[prev_str_num]);
    }
    free (str);
    command_line();
}

void    shell_command_executor(job work, program* prog)
{
    switch (prog->name)
    {
        case EXIT:
            shell_exit(work, prog);
            break;
        case CD:
            shell_cd(prog);
            break;
        case PWD:
            shell_pwd(prog);
            break;
        case HISTORY:
            shell_history(prog);
            break;
        default:
            break;
    }
}

int     find_redirection(char* str, int transfer, int str_size, program* prog)
{
    int     i, j, slash_count;

    for (i = 0; i <= prog->count_arg; i++)
    {
        if (str[transfer] != '\'' && str[transfer] != '"')
        {
            j = 0;
            while (str[transfer] != '\0')
            {
                if (str[transfer] == '>')
                {
                    if (transfer > 0 && str[transfer-1] == '\\')
                    {
                        slash_count = 1;
                        while (str[transfer - slash_count] == '\\')
                        {
                            slash_count++;
                            if (i - slash_count < 0)
                                break;
                        }
                        slash_count--;
                        transfer = str_slasher(str, transfer - slash_count, '>', str_size);
                        transfer--;
                        //printf ("slash_count: %d str: %s\n", slash_count, &str[0]);
                        if ((slash_count % 2) == 0)
                        {
                            str[transfer] = '\0';
                            transfer++;
                            if (str[transfer] == '>')
                            {
                                prog->output_type = APPEND;
                                transfer++;
                            }
                            else
                                prog->output_type = REWRITE;
                            transfer += str_transfer(str, str_size, transfer);
                            prog->output_file = (char*) malloc ((strlen (&str[transfer]) + 1)*sizeof (char));
                            strcpy (prog->output_file, &str[transfer]);
                            //prog->output_file = &str[transfer];
                            if (i != 0 && j == 0)
                                prog->count_arg = i - 1;
                            break;
                        }
                        str_size += (strlen (&str[transfer]) - (str_size - (transfer + 1))); 
                    }
                    else
                    {
                        str[transfer] = '\0';
                        transfer++;
                        if (str[transfer] == '>')
                        {
                            prog->output_type = APPEND;
                            transfer++;
                        }
                        else
                            prog->output_type = REWRITE;
                        transfer += str_transfer(str, str_size, transfer);
                        prog->output_file = (char*) malloc ((strlen (&str[transfer]) + 1)*sizeof (char));
                        strcpy (prog->output_file, &str[transfer]);
                        //prog->output_file = &str[transfer];
                        if (i != 0 && j == 0)
                            prog->count_arg = i - 1;
                        break; 
                    }
                }


                if (str[transfer] == '<')
                {
                    str[transfer] = '\0';
                    transfer++;
                    transfer += str_transfer(str, str_size, transfer);
                    prog->input_file = (char*) malloc ((strlen (&str[transfer]) + 1) * sizeof (char));
                    strcpy (prog->input_file, &str[transfer]);
                    //prog->input_file = &str[transfer];
                    prog->output_type = NO;
                    if ((prog->count_arg - i) > 0)
                        transfer += strlen (&str[transfer]) + 1;
                    while (str[transfer] != '\0')
                    {
                        if (str[transfer] == '>')
                        {
                            str[transfer] = '\0';
                            transfer++;
                            if (str[transfer] == '>')
                            {
                                str[transfer] = '\0';
                                transfer++;
                                transfer += str_transfer(str, str_size, transfer);
                                prog->output_type = APPEND;
                                prog->output_file = (char*) malloc ((strlen (&str[transfer]) + 1)*sizeof (char));
                                strcpy (prog->output_file, &str[transfer]);
                                //prog->output_file = &str[transfer];
                            }
                            else
                            {
                                transfer += str_transfer(str, str_size, transfer);
                                prog->output_type = REWRITE;
                                prog->output_file = (char*) malloc ((strlen (&str[transfer]) + 1)*sizeof (char));
                                strcpy (prog->output_file, &str[transfer]);
                                //prog->output_file = &str[transfer];
                            }
                            break;
                        }
                        transfer++;
                    }
                    if (i != 0 && j == 0)
                        prog->count_arg = i - 1;
                    break;
                }
                j++;
                transfer++;
            }
        }
        else
            transfer += strlen (&str[transfer]) + 1;
        transfer += str_transfer(str, str_size, transfer);
    }
    //printf ("str find: %d\n", prog->count_arg);
    return (str_size);
}

void    run_program(job work, program* prog)
{
    int     fd[2], file, flags;
    int     count;
    char*   string;

    if (prog->input_file == NULL && prog->output_file == NULL)
    {
        if (prog->name != NONE)
            shell_command_executor(work, prog);
        else
            command_another(prog);
    }
    else
    {
        pipe (fd);
        if ((fork ()) == 0)
        {
            if (prog->input_file != NULL)
            {
                file = open (prog->input_file, O_RDONLY, S_IRWXU);
                dup2 (file, 0);
                close (file);
            }
            close (fd[0]);
            if (prog->output_file != NULL)
                dup2 (fd[1], 1);
            close (fd[1]);
            if (prog->name != NONE)
                shell_command_executor(work, prog);
            else
                command_another(prog);
            exit (0);
        }
        close (fd[1]);
        if (prog->output_file != NULL)
        {
            switch (prog->output_type)
            {
                case REWRITE:
                    remove (prog->output_file);
                    flags = O_WRONLY | O_CREAT;
                    break;
                case APPEND:
                    flags = O_APPEND | O_WRONLY | O_CREAT;
                    break;
                default:
                    break;
            }
            file = open (prog->output_file, flags, S_IRWXU);
            count = 1;
            string = (char*) malloc (count * sizeof (char));
            while ((read(fd[0], &string[count-1], 1)))
            {
                count++;
                string = (char*) realloc (string, count * sizeof (char));
            }
            close (fd[0]);
            write (file, string, (count-1) * sizeof (char));
            close (file);
            free (string);
        }
        else
            close (fd[0]);
        wait (NULL);
    }
}

void    conv_executor(job work)
{
    pid_t   pid;
    int     fd[2], prev_fd[2], status, j;

    j = 0;
    while (j < work.count_prog)
    {
        if (j != work.count_prog - 1)
            pipe (fd);
        if ((pid = fork()) == 0)
        {
            if (j == 0)
            {
                dup2 (fd[1], 1);
                close (fd[1]);
                close (fd[0]);
                run_program(work, work.prog[j]);
                //execvp (work.prog[j]->arg[0], work.prog[j]->arg);
                exit (0);
            }
            else
            {
                if (j != work.count_prog - 1)
                {
                    dup2 (fd[1], 1);
                    dup2 (prev_fd[0], 0);
                    close (fd[1]);
                    close (prev_fd[0]);
                    close (fd[0]);
                    run_program(work, work.prog[j]);
                    //execvp (work.prog[j]->arg[0], work.prog[j]->arg);
                    exit (0);
                }
                else
                {
                    dup2 (prev_fd[0], 0);
                    close (prev_fd[0]);
                    //execvp (work.prog[j]->arg[0], work.prog[j]->arg);
                    run_program(work, work.prog[j]);
                    exit (0);
                }
            }
        }
        if (pid > 0) //Родитель
        {
            close (fd[1]);
            if (j != work.count_prog - 1)
            {
                if (j != 0)
                    close (prev_fd[0]);
                prev_fd[0] = fd[0];
            }
            else
                close (fd[0]);
        }
        if (pid == -1)
            printf ("%s\n", strerror(errno));
        waitpid (pid, &status, 0);
        j++;
    }
}

int     str_conv(char* str)
{
    int     i, count, quote_indicator, solo_quote_indicator, str_size, slash_count;

    i = 0;
    count = 1;
    solo_quote_indicator = quote_indicator = FALSE;
    if (strchr (str, '|'))
    {
        str_size = strlen (str);
        while (str[i] != '\0')
        {
            if (str[i] == '\'' && quote_indicator == FALSE)
            {
                if (solo_quote_indicator == TRUE)
                    solo_quote_indicator = FALSE;
                else
                    solo_quote_indicator = TRUE;
            }
            if (str[i] == '"' && solo_quote_indicator == FALSE)
            {
                if (quote_indicator == TRUE)
                    quote_indicator = FALSE;
                else
                    quote_indicator = TRUE;
            }
            if (str[i] == '|' && solo_quote_indicator == FALSE)
            {
                if (i > 0 && str[i-1] == '\\')
                {
                    slash_count = 1;
                    while (str[i-slash_count] == '\\')
                    {
                        slash_count++;
                        if (i-slash_count < 0)
                            break;
                    }
                    slash_count--;
                    if (quote_indicator == FALSE)
                    {
                        i = str_slasher(str, i-slash_count, '|', str_size);
                        i--;
                    }
                    else
                        i = quote_str_slasher(str, i-slash_count, '|', str_size);
                    //i--;
                    //printf ("i: %d str: %s\n",i, &str[prev_str_num]);
                    if (!(slash_count % 2) && quote_indicator == FALSE)
                    {
                        str[i] = '\0';
                        count++;
                    }
                    str_size += (strlen (&str[i]) - (str_size - (i + 1))); 
                }
                else
                {
                    if (quote_indicator == FALSE)
                    {
                        str[i] = '\0';
                        count++;
                    }
                }
            }
            i++;
        }
    }
    return (count);
}

void    do_job(job work)
{
    if (work.count_prog == 1)
        run_program(work, work.prog[0]);
       
    else
        conv_executor(work);
    free_job(work);
}

void    command_parser(char* str)
{
    int     count_cmd_all, slash_count, count_arg, count_progs, j, quote_indicator, solo_quote_indicator, i, space_indicator, str_size, transfer;//str_size
    //char*   conv_str;
    char*   conv_str_now;
    //program prog; //= {{NULL, NULL}, 0, NULL, NULL, NULL, NULL};
    job     work;

    j = 0;
    str_size = 0;
    count_cmd_all = sizeof (cmd_all) / sizeof (char*);
    //printf ("str: %s\n", str);
    count_progs = str_conv(str);
    conv_str_now = str;
    // printf ("%d\n", count_progs);
    
    work.count_prog = count_progs;
    work.prog = (program**) malloc (count_progs * sizeof (program*));
    while (j < count_progs)
    {
        work.prog[j] = (program*) malloc (sizeof (program));
        work.prog[j]->input_file = work.prog[j]->output_file = NULL;
        if (str_size)
            conv_str_now = &conv_str_now[str_size + 1];
        str = conv_str_now;
        i = transfer = count_arg = 0;
        str_size = strlen (str);
        quote_indicator = solo_quote_indicator = space_indicator = FALSE;
        if (str[0] == ' ')
            count_arg--;
        while (str[i] != '\0')
        {
            if (str[i] == '"' && solo_quote_indicator == FALSE)
            {
                if (i > 0 && str[i-1] == '\\')
                {
                    slash_count = 1;
                    while (str[i-slash_count] == '\\')
                    {
                        slash_count++;
                        if (i-slash_count < 0)
                            break;
                    }
                    slash_count--;
                    i = str_slasher(str, i-slash_count, '"', str_size);
                    i--;
                    if ((slash_count % 2) == 0)
                    {
                        str[i] = '\0';
                        if (quote_indicator == FALSE)
                            quote_indicator = TRUE;
                        else
                            quote_indicator = FALSE;  
                    }
                    str_size += (strlen (&str[i]) - (str_size - (i + 1))); 
                }
                else
                {
                    str[i] = '\0';
                    if (quote_indicator == FALSE)
                        quote_indicator = TRUE;
                    else
                        quote_indicator = FALSE;
                }
            }
            if (str[i] == '\'' && quote_indicator == FALSE)
            {
                if (i > 0 && str[i-1] == '\\')
                {
                    slash_count = 1;
                    while (str[i-slash_count] == '\\')
                    {
                        slash_count++;
                        if (i-slash_count < 0)
                            break;
                    }
                    slash_count--;
                    i = str_slasher(str, i-slash_count, '\'', str_size);
                    i--;
                    if ((slash_count % 2) == 0)
                    {
                        str[i] = '\0';
                        if (solo_quote_indicator == FALSE)
                            solo_quote_indicator = TRUE;
                        else
                            solo_quote_indicator = FALSE;  
                    }
                    str_size += (strlen (&str[i]) - (str_size - (i + 1))); 
                }
                else
                {
                    str[i] = '\0';
                    if (solo_quote_indicator == FALSE)
                        solo_quote_indicator = TRUE;
                    else
                        solo_quote_indicator = FALSE;
                }
            }

            if (str[i] != ' ' && space_indicator == TRUE)
            {
                space_indicator = FALSE;
                count_arg++;
            }
            if (str[i] == ' ' && quote_indicator == FALSE && solo_quote_indicator == FALSE)
            {
                str[i] = '\0';
                if (space_indicator == FALSE)
                    space_indicator = TRUE;
            }
            i++;
        }
        work.prog[j]->count_arg = count_arg;
        //Распознание команды
        while (transfer <= str_size)
        {
            if (str[transfer] != '\0')
                break;
            else
                transfer++;
        }
        work.prog[j]->name = NONE;


        str_size = find_redirection(str, transfer, str_size, work.prog[j]);

        for (i = 0;i < count_cmd_all; i++)
        {
            if (!strcmp (&str[transfer], cmd_all[i]))
            {
                work.prog[j]->name = i + 1; //т.к. NONE = 0
                // cmd_body.error = OK;
                // command_size = strlen (&str[transfer]);
                transfer += (strlen (&str[transfer]) + 1);
                break;
            }
        }
        if ((work.prog[j]->count_arg > 0 && work.prog[j]->name != NONE) || (work.prog[j]->count_arg >= 0 && work.prog[j]->name == NONE))
        {
            //При NONE нулевой аргумент в prog.cout_arg не учитывается
            if (work.prog[j]->name == NONE)
                work.prog[j]->count_arg++;
            work.prog[j]->arg = (char**) malloc ((work.prog[j]->count_arg + 1) * sizeof (char*));
            for (i = 0; i < work.prog[j]->count_arg; i++)
            {
                transfer += str_transfer(str, str_size, transfer);
                work.prog[j]->arg[i] = &str[transfer];
                work.prog[j]->arg[i] = (char*) malloc ((strlen (&str[transfer]) + 1) * sizeof (char));
                strcpy (work.prog[j]->arg[i], &str[transfer]);
                //printf ("i = %d str: %s\n", i,  work.prog[j]->arg[i]);
                transfer += strlen (&str[transfer]) + 1;
            }
            if (work.prog[j]->name == NONE)
            {
                work.prog[j]->arg[i] = NULL;
                //printf ("\nstr: i = %d %s", i,  work.prog[j]->arg[i]);
            }
            //printf ("\nfile = %s\n", work.prog[j]->output_file);
            //printf ("\ncount: arg = %d\n", work.prog[j]->count_arg);
        }
        j++;
    }
    //printf ("count progs = %d", work.count_prog);
    //Запуск всех програм из работы
    //j = 0;
    do_job(work);
    // if (work.count_prog == 1)
    //     run_program(work.prog[j]);
    // else
    //     conv_executor(work);
}



int     main(int argc, char** argv)
{
    printf ("\e[2J\e[3J\e[r");
    argc_main = argc;
    argv_main = argv;
    srand (getpid () + argc);
    printf ("ShellDone v1.%d\n", rand());
    special_value(argv[0]);
    command_line();
}