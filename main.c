#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <signal.h>

const double frequencies[256];
const double digram_frequencies[256][256];

typedef int char_state[256];

double chi_squared_char(char_state counts, size_t length)
{
	double chi2 = 0.0;
	for (size_t i = 0; i < 256; i++) {
		/* so we don't divide by zero. */
		double freq = frequencies[i] ? frequencies[i] : FLT_MIN;
		double expected = ((double) length) * freq;
		double error = expected - counts[i];
		chi2 += (error * error) / expected;
	}
	return chi2;
}

typedef struct {
	int counts[256][256];
	char last_char;
} digram_state;

double chi_squared_digram(digram_state *state, size_t length)
{
	double chi2 = 0.0;
	for (size_t i = 0; i < 256; i++) {
		for (size_t j = 0; j < 256; j++) {
			/* so we don't divide by zero. */
			double freq = digram_frequencies[i][j] ? digram_frequencies[i][j] : FLT_MIN;
			double expected = ((double) length) * freq;
			double error = expected - state->counts[i][j];
			chi2 += (error * error) / expected;
		}
	}
	return chi2;
}

int process_char(char_state counts, char x)
{
	counts[(int) x] += 1;
	return 0;
}

void process_digram(digram_state *state, char here)
{
	if (state->last_char)
		state->counts[(int) state->last_char][(int) here] += 1;
	state->last_char = here;
}

void handle_input(int is_digram_mode, int in, int out)
{
	char here = 0;
	if (is_digram_mode) {
		size_t length = 0;
		digram_state state = {0};
		while (read(in, &here, sizeof(char)) > 0) {
			length += 1;
			process_digram(&state, here);
		}
		if (length)
			dprintf(out, "%f\n", chi_squared_digram(&state, length));
	} else {
		size_t length = 0;
		char_state state = {0};
		while (read(in, &here, sizeof(char)) > 0) {
			length += 1;
			process_char(state, here);
		}
		if (length)
			dprintf(out, "%f\n", chi_squared_char(state, length));
	}
}

int should_continue = 1;

void handle_signal(int _) {
	should_continue = 0;
}

int main (int argc, char **argv)
{
	int is_digram_mode = 0;
	int has_socket = 0;
	/* max sun_path for unix sockets */
	char socket_path[108] = {0};
	int option = 0;
	opterr = 0;
	while ((option = getopt(argc, argv, "n:s:")) != -1) {
		switch (option) {
		case 'n':
			if (!strcmp(optarg, "1")) {
				is_digram_mode = 0;
			} else if (!strcmp(optarg, "2")) {
				is_digram_mode = 1;
			} else {
				goto usage;
			}
			break;
		case 's':
			has_socket = 1;
			if (strlen(optarg) + 1 > sizeof(socket_path)) {
				fprintf(stderr, "=> socket path too long!\n");
				return -1;
			}
			memcpy(socket_path, optarg, sizeof(socket_path));
			break;
		default:
			goto usage;
		}
	}
	if (has_socket) {
		if (signal(SIGINT, handle_signal) == SIG_ERR
		    || signal(SIGTERM, handle_signal ) == SIG_ERR) {
			fprintf(stderr, "(while setting a signal handler): %m\n");
			return -1;
		}

		int in = 0;
		if ((in = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			fprintf(stderr, "(while trying to open a socket): %m\n");
			return -1;
		}
		struct sockaddr_un addr = { .sun_family = AF_UNIX };
		memcpy(&addr.sun_path, socket_path, 108);
		if (bind(in, (const struct sockaddr *) &addr, sizeof(addr))) {
			fprintf(stderr, "(while trying to bind %s): %m\n", socket_path);
			close(in);
			return -1;
		}
		if (listen(in, 64)) {
			fprintf(stderr, "(while trying to listen): %m\n");
			close(in);
			unlink(socket_path);
			return -1;
		}
		if (fcntl(in, F_SETFL, O_NONBLOCK)) {
			fprintf(stderr, "(while setting the socket nonblocking): %m\n");
			close(in);
			unlink(socket_path);
			return -1;
		}
		size_t num_clients = 0;
		struct pollfd clients[64] = {0};
		while (should_continue) {
			/* deal with existing clients. */
			int nevents = 0;
			if ((nevents = poll(clients, num_clients, 200))) {
				for (size_t i = 0; i < num_clients; i++) {
					if (clients[i].revents & (POLLERR | POLLHUP)) {
						close(clients[i].fd);
						/* compact. */
						if (i < num_clients - 1) {
							memcpy(&clients[i],
							       &clients[num_clients - 1],
							       sizeof(struct pollfd));
							memcpy(&clients[num_clients - 1],
							       &(struct pollfd) {0},
							       sizeof(struct pollfd));
						} else {
							memcpy(&clients[i],
							       &(struct pollfd) {0},
							       sizeof(struct pollfd));
						}
						/* underflow isn't UB here since size_t is unsigned.
						   after this iteration we'll add 1 anyway...*/
						i -= 1;
						num_clients -= 1;
					} else if (clients[i].revents & POLLIN) {
						handle_input(is_digram_mode, clients[i].fd, clients[i].fd);
						close(clients[i].fd);
					}
				}
			}
			/* check for new clients. */
			int new_fd = 0;
			if (num_clients < 64 &&
			    (new_fd = accept(in, NULL, 0)) > 0) {
				if (fcntl(new_fd, F_SETFL, 0)) {
					fprintf(stderr, "(while trying to set fd #%d non-blocking): %m\n", new_fd);
					close(new_fd);
					continue;
				}
				memcpy(&clients[num_clients++],
				       &(struct pollfd) {
					       .fd = new_fd,
					       .events = POLLIN | POLLERR | POLLHUP
				       },
				       sizeof(struct pollfd));
			}
		}
		close(in);
		unlink(socket_path);
		for (size_t i = 0; i < num_clients; i++)
			close(clients[i].fd);
	} else {
		handle_input(is_digram_mode, STDIN_FILENO, STDOUT_FILENO);
	}
	return 0;
usage:
	fprintf(stderr,
		"usage: \n"
		"$ printf \"data\" | %s [-n 1 | -n 2)]\n"
		"16.883203\n"
		"$\n\n"
		"or:\n"
		"$ printf \"a\\0b\\0\" | %s -p [-n 1 | -n 2]\n"
		"0.985068\n"
		"0.985068\n"
		"$\n",
		argv[0],
		argv[0]);
	return 1;
}
