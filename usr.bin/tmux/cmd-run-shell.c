/* $OpenBSD: cmd-run-shell.c,v 1.77 2021/08/23 12:33:55 nicm Exp $ */

/*
 * Copyright (c) 2009 Tiago Cunha <me@tiagocunha.org>
 * Copyright (c) 2009 Nicholas Marriott <nicm@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "tmux.h"

/*
 * Runs a command without a window.
 */

static enum cmd_retval	cmd_run_shell_exec(struct cmd *, struct cmdq_item *);

static void	cmd_run_shell_timer(int, short, void *);
static void	cmd_run_shell_callback(struct job *);
static void	cmd_run_shell_free(void *);
static void	cmd_run_shell_print(struct job *, const char *);

const struct cmd_entry cmd_run_shell_entry = {
	.name = "run-shell",
	.alias = "run",

	.args = { "bd:Ct:", 0, 1, NULL },
	.usage = "[-bC] [-d delay] " CMD_TARGET_PANE_USAGE " [shell-command]",

	.target = { 't', CMD_FIND_PANE, CMD_FIND_CANFAIL },

	.flags = 0,
	.exec = cmd_run_shell_exec
};

struct cmd_run_shell_data {
	struct client		*client;
	char			*cmd;
	struct cmd_list		*cmdlist;
	char			*cwd;
	struct cmdq_item	*item;
	struct session		*s;
	int			 wp_id;
	struct event		 timer;
	int			 flags;
};

static void
cmd_run_shell_print(struct job *job, const char *msg)
{
	struct cmd_run_shell_data	*cdata = job_get_data(job);
	struct window_pane		*wp = NULL;
	struct cmd_find_state		 fs;
	struct window_mode_entry	*wme;

	if (cdata->wp_id != -1)
		wp = window_pane_find_by_id(cdata->wp_id);
	if (wp == NULL) {
		if (cdata->item != NULL) {
			cmdq_print(cdata->item, "%s", msg);
			return;
		}
		if (cmd_find_from_nothing(&fs, 0) != 0)
			return;
		wp = fs.wp;
		if (wp == NULL)
			return;
	}

	wme = TAILQ_FIRST(&wp->modes);
	if (wme == NULL || wme->mode != &window_view_mode)
		window_pane_set_mode(wp, NULL, &window_view_mode, NULL, NULL);
	window_copy_add(wp, "%s", msg);
}

static enum cmd_retval
cmd_run_shell_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args			*args = cmd_get_args(self);
	struct cmd_find_state		*target = cmdq_get_target(item);
	struct cmd_run_shell_data	*cdata;
	struct client			*tc = cmdq_get_target_client(item);
	struct session			*s = target->s;
	struct window_pane		*wp = target->wp;
	const char			*delay, *cmd;
	double				 d;
	struct timeval			 tv;
	char				*end;
	int				 wait = !args_has(args, 'b');

	if ((delay = args_get(args, 'd')) != NULL) {
		d = strtod(delay, &end);
		if (*end != '\0') {
			cmdq_error(item, "invalid delay time: %s", delay);
			return (CMD_RETURN_ERROR);
		}
	} else if (args_count(args) == 0)
		return (CMD_RETURN_NORMAL);

	cdata = xcalloc(1, sizeof *cdata);
	if (!args_has(args, 'C')) {
		cmd = args_string(args, 0);
		if (cmd != NULL)
			cdata->cmd = format_single_from_target(item, cmd);
	} else {
		cdata->cmdlist = args_make_commands_now(self, item, 0);
		if (cdata->cmdlist == NULL)
			return (CMD_RETURN_ERROR);
	}

	if (args_has(args, 't') && wp != NULL)
		cdata->wp_id = wp->id;
	else
		cdata->wp_id = -1;

	if (wait) {
		cdata->client = cmdq_get_client(item);
		cdata->item = item;
	} else {
		cdata->client = tc;
		cdata->flags |= JOB_NOWAIT;
	}
	if (cdata->client != NULL)
		cdata->client->references++;

	cdata->cwd = xstrdup(server_client_get_cwd(cmdq_get_client(item), s));

	cdata->s = s;
	if (s != NULL)
		session_add_ref(s, __func__);

	evtimer_set(&cdata->timer, cmd_run_shell_timer, cdata);
	if (delay != NULL) {
		timerclear(&tv);
		tv.tv_sec = (time_t)d;
		tv.tv_usec = (d - (double)tv.tv_sec) * 1000000U;
		evtimer_add(&cdata->timer, &tv);
	} else
		event_active(&cdata->timer, EV_TIMEOUT, 1);

	if (!wait)
		return (CMD_RETURN_NORMAL);
	return (CMD_RETURN_WAIT);
}

static void
cmd_run_shell_timer(__unused int fd, __unused short events, void* arg)
{
	struct cmd_run_shell_data	*cdata = arg;
	struct client			*c = cdata->client;
	const char			*cmd = cdata->cmd;
	struct cmdq_item		*item = cdata->item, *new_item;

	if (cdata->cmdlist == NULL && cmd != NULL) {
		if (job_run(cmd, 0, NULL, cdata->s, cdata->cwd, NULL,
		    cmd_run_shell_callback, cmd_run_shell_free, cdata,
		    cdata->flags, -1, -1) == NULL)
			cmd_run_shell_free(cdata);
		return;
	}

	if (cdata->cmdlist != NULL) {
		if (item == NULL) {
			new_item = cmdq_get_command(cdata->cmdlist, NULL);
			cmdq_append(c, new_item);
		} else {
			new_item = cmdq_get_command(cdata->cmdlist,
			    cmdq_get_state(item));
			cmdq_insert_after(item, new_item);
		}
	}

	if (cdata->item != NULL)
		cmdq_continue(cdata->item);
	cmd_run_shell_free(cdata);
}

static void
cmd_run_shell_callback(struct job *job)
{
	struct cmd_run_shell_data	*cdata = job_get_data(job);
	struct bufferevent		*event = job_get_event(job);
	struct cmdq_item		*item = cdata->item;
	char				*cmd = cdata->cmd, *msg = NULL, *line;
	size_t				 size;
	int				 retcode, status;

	do {
		if ((line = evbuffer_readline(event->input)) != NULL) {
			cmd_run_shell_print(job, line);
			free(line);
		}
	} while (line != NULL);

	size = EVBUFFER_LENGTH(event->input);
	if (size != 0) {
		line = xmalloc(size + 1);
		memcpy(line, EVBUFFER_DATA(event->input), size);
		line[size] = '\0';

		cmd_run_shell_print(job, line);

		free(line);
	}

	status = job_get_status(job);
	if (WIFEXITED(status)) {
		if ((retcode = WEXITSTATUS(status)) != 0)
			xasprintf(&msg, "'%s' returned %d", cmd, retcode);
	} else if (WIFSIGNALED(status)) {
		retcode = WTERMSIG(status);
		xasprintf(&msg, "'%s' terminated by signal %d", cmd, retcode);
		retcode += 128;
	} else
		retcode = 0;
	if (msg != NULL)
		cmd_run_shell_print(job, msg);
	free(msg);

	if (item != NULL) {
		if (cmdq_get_client(item) != NULL &&
		    cmdq_get_client(item)->session == NULL)
			cmdq_get_client(item)->retval = retcode;
		cmdq_continue(item);
	}
}

static void
cmd_run_shell_free(void *data)
{
	struct cmd_run_shell_data	*cdata = data;

	evtimer_del(&cdata->timer);
	if (cdata->s != NULL)
		session_remove_ref(cdata->s, __func__);
	if (cdata->client != NULL)
		server_client_unref(cdata->client);
	if (cdata->cmdlist != NULL)
		cmd_list_free(cdata->cmdlist);
	free(cdata->cwd);
	free(cdata->cmd);
	free(cdata);
}
