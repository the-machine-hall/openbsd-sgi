#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define UV_SHOULD_SUCCEED(A, B) do {					\
	if (A) {							\
		err(1, "%s:%d - %s", __FILE__, __LINE__, B);		\
	}								\
} while (0)

#define NUM_PERMS 16
static char uv_dir[] = "/tmp/uvdir.XXXXXX"; /* test directory */
static char uv_file[] = "/tmp/uvfile.XXXXXX"; /* log file */

const char* perms[] = {"", "r", "w", "x", "c", "rw", "rx", "rc",
                       "wx", "wc","xc", "rwx", "rwc", "rxc", "wxc", "rwxc"};
const char* filenames[] = {"f", "fr", "fw", "fx", "fc", "frw", "frx", "frc",
			   "fwx", "fwc", "fxc", "frwx",
			   "frwc", "frxc", "fwxc", "frwxc"};
const char* header = "unveil:access\n";

int main(int argc, char** argv) {
	int i;
	int log_fd;
	FILE *log;
	char expected[PATH_MAX];
	char *exp;

	UV_SHOULD_SUCCEED(((exp = realpath("access-expected", expected)) == NULL), "realpath");
	UV_SHOULD_SUCCEED(((log_fd = mkstemp(uv_file)) == -1), "mkstemp");
	UV_SHOULD_SUCCEED(((log = fdopen(log_fd, "w")) == NULL), "fdopen");
	UV_SHOULD_SUCCEED((mkdtemp(uv_dir) == NULL), "mkdtmp");
	UV_SHOULD_SUCCEED((unveil("/", "rwxc") == -1), "unveil");
	UV_SHOULD_SUCCEED((chdir(uv_dir) == -1), "chdir");

	fwrite(header, strlen(header), 1, log);
	for (i = 0; i < NUM_PERMS; i++) {
		const char *perm = perms[i];
		const char *filename = filenames[i];
		int fd;
		UV_SHOULD_SUCCEED(((fd = open(filename, O_WRONLY|O_CREAT, 0700)) == -1), "open");
		UV_SHOULD_SUCCEED((close(fd) == -1), "close");
		UV_SHOULD_SUCCEED((unveil(filename, perm) == -1), "unveil");
		UV_SHOULD_SUCCEED((fwrite(perm, 1, strlen(perm), log) != strlen(perm)), "fwrite");
		UV_SHOULD_SUCCEED((fwrite(":", 1, 1, log) != 1), "fwrite");
		if (access(filename, R_OK) == 0)
			UV_SHOULD_SUCCEED((fwrite("R", 1, 1, log) != 1), "fwrite");
		if (access(filename, W_OK) == 0)
			UV_SHOULD_SUCCEED((fwrite("W", 1, 1, log) != 1), "fwrite");
		if (access(filename, X_OK) == 0)
			UV_SHOULD_SUCCEED((fwrite("X", 1, 1, log) != 1), "fwrite");
		if (access(filename, F_OK) == 0)
			UV_SHOULD_SUCCEED((fwrite("F", 1, 1, log) != 1), "fwrite");
		UV_SHOULD_SUCCEED((fwrite("\n", 1, 1, log) != 1), "fwrite");
	}
	UV_SHOULD_SUCCEED((fclose(log) == -1), "fclose");

	return execl("/usr/bin/diff", "diff", "-u", uv_file, expected, NULL);
}
