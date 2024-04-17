/**
 * Q2Admin
 * ref/admin related stuff
 */

#include "g_local.h"

admin_type admin_pass[MAX_ADMINS];
admin_type q2a_bypass_pass[MAX_ADMINS];
int num_admins = 0;
int num_q2a_admins = 0;

/**
 *
 */
void Read_Admin_cfg(void) {
    FILE *f;
    char name[256];
    int i, i2;

    Q_snprintf(name, sizeof(name), "%s/%s", moddir, configfile_login->string);

    f = fopen(name, "rb");
    if (!f) {
        gi.dprintf("WARNING: %s could not be found\n", name);
        goto file2;
        return;
    }

    i = 0;
    while ((!feof(f)) && (i < MAX_ADMINS)) {
        fscanf(f, "%s %s %d", &admin_pass[i].name, &admin_pass[i].password, &admin_pass[i].level);
        i++;
    }
    if (!admin_pass[i].level)
        i--;
    num_admins = i;
    if (i < MAX_ADMINS)
        for (i2 = i; i2 < MAX_ADMINS; i2++)
            admin_pass[i2].level = 0;

    //read em in
    fclose(f);

file2:
    ;
    Q_snprintf(name, sizeof(name), "%s/%s", moddir, configfile_bypass->string);

    f = fopen(name, "rb");
    if (!f) {
        gi.dprintf("WARNING: %s could not be found\n", name);
        return;
    }

    i = 0;
    while ((!feof(f)) && (i < MAX_ADMINS)) {
        fscanf(f, "%s %s %d", &q2a_bypass_pass[i].name, &q2a_bypass_pass[i].password, &q2a_bypass_pass[i].level);
        i++;
    }
    if (!q2a_bypass_pass[i].level)
        i--;
    num_q2a_admins = i;
    if (i < MAX_ADMINS)
        for (i2 = i; i2 < MAX_ADMINS; i2++)
            q2a_bypass_pass[i2].level = 0;

    //read em in
    fclose(f);
}

/**
 *
 */
void List_Admin_Commands(edict_t *ent, int client) {
    if (proxyinfo[client].q2a_admin & 1) {
        gi.cprintf(ent, PRINT_HIGH, "    - !boot <number>\n");
    }
    if (proxyinfo[client].q2a_admin & 2) {
        gi.cprintf(ent, PRINT_HIGH, "    - !dumpmsec\n");
    }
    if (proxyinfo[client].q2a_admin & 4) {
        gi.cprintf(ent, PRINT_HIGH, "    - !changemap <mapname>\n");
    }
    if (proxyinfo[client].q2a_admin & 8) {
        gi.cprintf(ent, PRINT_HIGH, "    - !dumpuser <num>\n");
    }
    if (proxyinfo[client].q2a_admin & 16) {
        gi.cprintf(ent, PRINT_HIGH, "    - !auth\n");
        gi.cprintf(ent, PRINT_HIGH, "    - !gfx\n");
    }
    if (proxyinfo[client].q2a_admin & 32) {
        gi.cprintf(ent, PRINT_HIGH, "    - !dostuff <num> <commands>\n");
    }
    if (proxyinfo[client].q2a_admin & 128) {
        if (whois_active)
            gi.cprintf(ent, PRINT_HIGH, "    - !writewhois\n");
    }
    gi.cprintf(ent, PRINT_HIGH, "\n");
}

/**
 *
 */
void reloadLoginFileRun(int startarg, edict_t *ent, int client) {
    Read_Admin_cfg();
    gi.cprintf(ent, PRINT_HIGH, "Login file reloaded.\n");
}

/**
 *
 */
int get_bypass_level(char *givenpass, char *givenname) {
    int got_level = 0;
    unsigned int i;

    for (i = 0; i < num_q2a_admins; i++) {
        if (!q2a_bypass_pass[i].level)
            break;
        if ((strcmp(givenpass, q2a_bypass_pass[i].password) == 0) && (strcmp(givenname, q2a_bypass_pass[i].name) == 0)) {
            got_level = q2a_bypass_pass[i].level;
            break;
        }
    }
    return got_level;
}

/**
 *
 */
int get_admin_level(char *givenpass, char *givenname) {
    int got_level = 0;
    unsigned int i;

    for (i = 0; i < num_admins; i++) {
        if (!admin_pass[i].level)
            break;
        if ((strcmp(givenpass, admin_pass[i].password) == 0) && (strcmp(givenname, admin_pass[i].name) == 0)) {
            got_level = admin_pass[i].level;
            break;
        }
    }
    return got_level;
}

/**
 *
 */
void ADMIN_players(edict_t *ent, int client) {
    unsigned int i;
    gi.cprintf(ent, PRINT_HIGH, "Players\n");
    for (i = 0; i < maxclients->value; i++) {
        if (proxyinfo[i].inuse) {
            gi.cprintf(ent, PRINT_HIGH, "  %2i : %s\n", i, proxyinfo[i].name);
        }
    }
    gi.cprintf(ent, PRINT_HIGH, "*******************************\n");
}

/**
 *
 */
void ADMIN_dumpmsec(edict_t *ent, int client) {
    unsigned int i;
    gi.cprintf(ent, PRINT_HIGH, "MSEC\n");
    for (i = 0; i < maxclients->value; i++) {
        if (proxyinfo[i].inuse) {
            gi.cprintf(ent, PRINT_HIGH, "  %2i : %-16s %d\n", i, proxyinfo[i].name, proxyinfo[i].msec_last);
        }
    }
    gi.cprintf(ent, PRINT_HIGH, "*******************************\n");
}

/**
 *
 */
void ADMIN_dumpuser(edict_t *ent, int client, int user, qboolean check) {
    char *cp1;

    if (gi.argc() < 2) {
        ADMIN_players(ent, client);
        return;
    }

    if (check) {
        if (!proxyinfo[user].inuse) {
            return;
        }
    }
    //if (proxyinfo[user].inuse)
    {
        gi.cprintf(ent, PRINT_HIGH, "User Info for client %d\n", user);

        cp1 = Info_ValueForKey(proxyinfo[user].userinfo, "msg");
        gi.cprintf(ent, PRINT_HIGH, "msg          %s\n", cp1);

        cp1 = Info_ValueForKey(proxyinfo[user].userinfo, "spectator");
        gi.cprintf(ent, PRINT_HIGH, "spectator    %s\n", cp1);

        cp1 = Info_ValueForKey(proxyinfo[user].userinfo, "cl_maxfps");
        gi.cprintf(ent, PRINT_HIGH, "cl_maxfps    %s\n", cp1);

        cp1 = Info_ValueForKey(proxyinfo[user].userinfo, "gender");
        gi.cprintf(ent, PRINT_HIGH, "gender       %s\n", cp1);

        cp1 = Info_ValueForKey(proxyinfo[user].userinfo, "fov");
        gi.cprintf(ent, PRINT_HIGH, "fov          %s\n", cp1);

        cp1 = Info_ValueForKey(proxyinfo[user].userinfo, "rate");
        gi.cprintf(ent, PRINT_HIGH, "rate         %s\n", cp1);

        cp1 = Info_ValueForKey(proxyinfo[user].userinfo, "skin");
        gi.cprintf(ent, PRINT_HIGH, "skin         %s\n", cp1);

        cp1 = Info_ValueForKey(proxyinfo[user].userinfo, "hand");
        gi.cprintf(ent, PRINT_HIGH, "hand         %s\n", cp1);

        if (strlen(proxyinfo[user].gl_driver)) {
            gi.cprintf(ent, PRINT_HIGH, "gl_driver    %s\n", proxyinfo[user].gl_driver);
        }

        if (proxyinfo[client].q2a_admin & 16) {
            gi.cprintf(ent, PRINT_HIGH, "ip           %s\n", IP(user));
        }

        cp1 = Info_ValueForKey(proxyinfo[user].userinfo, "name");
        gi.cprintf(ent, PRINT_HIGH, "name         %s\n", cp1);

        if (proxyinfo[client].q2a_admin & 128) {
            gi.cprintf(ent, PRINT_HIGH, "full         %s\n", proxyinfo[user].userinfo);
        }
    }
}

/**
 *
 */
void ADMIN_auth(edict_t *ent) {
    unsigned int i;
    for (i = 0; i < maxclients->value; i++) {
        if (proxyinfo[i].inuse) {
            stuffcmd(getEnt((i + 1)), "say I'm using $version\n");
        }
    }
}

/**
 *
 */
void ADMIN_gfx(edict_t *ent) {
    unsigned int i;
    for (i = 0; i < maxclients->value; i++) {
        if (proxyinfo[i].inuse) {
            stuffcmd(getEnt((i + 1)), "say I'm using $gl_driver ( $vid_ref ) / $gl_mode\n");
        }
    }
}

/**
 *
 */
void ADMIN_boot(edict_t *ent, int client, int user) {
    char tmptext[100];
    if (gi.argc() < 2) {
        ADMIN_players(ent, client);
        return;
    }
    if ((user >= 0) && (user < maxclients->value)) {
        if (proxyinfo[user].inuse) {
            gi.bprintf(PRINT_HIGH, "%s was kicked by %s.\n", proxyinfo[user].name, proxyinfo[client].name);
            Q_snprintf(tmptext, sizeof(tmptext), "\nkick %d\n", user);
            gi.AddCommandString(tmptext);
        }
    }
}

/**
 *
 */
void ADMIN_changemap(edict_t *ent, int client, char *mname) {
    char tmptext[100];
    if (gi.argc() < 2) {
        ADMIN_players(ent, client);
        return;
    }
    if (q2a_strstr(mname, "\"")) {
        return;
    }
    if (q2a_strstr(mname, ";")) {
        return;
    }
    gi.bprintf(PRINT_HIGH, "%s is changing map to %s.\n", proxyinfo[client].name, mname);
    Q_snprintf(tmptext, sizeof(tmptext), "\nmap %s\n", mname);
    gi.AddCommandString(tmptext);
}

/**
 *
 */
int ADMIN_process_command(edict_t *ent, int client) {
    unsigned int i, done = 0;
    int send_to_client;
    edict_t *send_to_ent;
    char send_string[512];
    char abuffer[256];

    if (strlen(gi.args())) {
        Q_snprintf(abuffer, sizeof(abuffer), "COMMAND - %s %s", gi.argv(0), gi.args());
        logEvent(LT_ADMINLOG, client, ent, abuffer, 0, 0.0);
        gi.dprintf("%s\n", abuffer);
    }

    if (proxyinfo[client].q2a_admin & 1) {
        //Level 1 commands
        if (strcmp(gi.argv(0), "!boot") == 0) {
            ADMIN_boot(ent, client, atoi(gi.argv(1)));
            done = 1;
        }
    }

    if (proxyinfo[client].q2a_admin & 2) {
        //Level 2 commands
        if (strcmp(gi.argv(0), "!dumpmsec") == 0) {
            ADMIN_dumpmsec(ent, client);
            done = 1;
        }
    }
    if (proxyinfo[client].q2a_admin & 4) {
        //Level 3 commands
        if (strcmp(gi.argv(0), "!changemap") == 0) {
            ADMIN_changemap(ent, client, gi.argv(1));
            done = 1;
        }
    }
    if (proxyinfo[client].q2a_admin & 8) {
        //Level 4 commands
        if (strcmp(gi.argv(0), "!dumpuser") == 0) {
            ADMIN_dumpuser(ent, client, atoi(gi.argv(1)), qtrue);
            done = 1;
        } else if (strcmp(gi.argv(0), "!dumpuser_any") == 0) {
            ADMIN_dumpuser(ent, client, atoi(gi.argv(1)), qfalse);
            done = 1;
        }
    }
    if (proxyinfo[client].q2a_admin & 16) {
        //Level 5 commands
        if (strcmp(gi.argv(0), "!auth") == 0) {
            ADMIN_auth(ent);
            done = 1;
            gi.cprintf(ent, PRINT_HIGH, "A new auth command has been issued.\n");
        } else if (strcmp(gi.argv(0), "!gfx") == 0) {
            ADMIN_gfx(ent);
            done = 1;
            gi.cprintf(ent, PRINT_HIGH, "Graphics command issued.\n");
        }
    }

    if (proxyinfo[client].q2a_admin & 32) {
        //Level 7 commands

        if (strcmp(gi.argv(0), "!dostuff") == 0) {
            if (gi.argc() > 2) {
                send_to_client = atoi(gi.argv(1));
                if (strcmp(gi.argv(1), "all") == 0) {
                    for (send_to_client = 0; send_to_client < maxclients->value; send_to_client++)
                        if (proxyinfo[send_to_client].inuse) {
                            q2a_strncpy(send_string, gi.argv(2), sizeof(send_string));
                            if (gi.argc() > 3)
                                for (i = 3; i < gi.argc(); i++) {
                                    strcat(send_string, " ");
                                    strcat(send_string, gi.argv(i));
                                }
                            send_to_ent = getEnt((send_to_client + 1));
                            stuffcmd(send_to_ent, send_string);
                            gi.cprintf(ent, PRINT_HIGH, "Client %d (%s) has been stuffed!\n", send_to_client, proxyinfo[send_to_client].name);
                        }
                } else
                    if (proxyinfo[send_to_client].inuse) {
                    q2a_strncpy(send_string, gi.argv(2), sizeof(send_string));
                    if (gi.argc() > 3)
                        for (i = 3; i < gi.argc(); i++) {
                            strcat(send_string, " ");
                            strcat(send_string, gi.argv(i));
                        }
                    send_to_ent = getEnt((send_to_client + 1));
                    stuffcmd(send_to_ent, send_string);
                    gi.cprintf(ent, PRINT_HIGH, "Client %d (%s) has been stuffed!\n", send_to_client, proxyinfo[send_to_client].name);
                }
            }
            done = 2;
        }
    }

    if (proxyinfo[client].q2a_admin & 128) {
        if ((strcmp(gi.argv(0), "!writewhois") == 0) && (whois_active)) {
            whois_write_file();
            done = 1;
            gi.cprintf(ent, PRINT_HIGH, "Whois file written.\n");
        }
    }
    return done;
}
