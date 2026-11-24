NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
INCLUDES	= -Iincludes

SRC_DIR		= srcs
OBJ_DIR		= objs

SRCS		= $(SRC_DIR)/main.c \
			  $(SRC_DIR)/parsing/parse_args.c \
			  $(SRC_DIR)/parsing/parse_number.c \
			  $(SRC_DIR)/parsing/parse_scheduler.c \
			  $(SRC_DIR)/structures/sim_init.c \
			  $(SRC_DIR)/structures/sim_destroy.c \
			  $(SRC_DIR)/structures/coder_init.c \
			  $(SRC_DIR)/dongle/dongle_acquire.c \
			  $(SRC_DIR)/dongle/dongle_release.c \
			  $(SRC_DIR)/dongle/dongle_cooldown.c \
			  $(SRC_DIR)/dongle/dongle_queue.c \
			  $(SRC_DIR)/dongle/dongle_wait.c \
			  $(SRC_DIR)/scheduler/heap_core.c \
			  $(SRC_DIR)/scheduler/heap.c \
			  $(SRC_DIR)/scheduler/heap_utils.c \
			  $(SRC_DIR)/scheduler/deadline.c \
			  $(SRC_DIR)/coder/coder_routine.c \
			  $(SRC_DIR)/coder/coder_cycle.c \
			  $(SRC_DIR)/monitor/monitor.c \
			  $(SRC_DIR)/logging/log.c \
			  $(SRC_DIR)/utils/time_utils.c \
			  $(SRC_DIR)/utils/sim_stop.c \
			  $(SRC_DIR)/utils/coder_sleep.c \
			  $(SRC_DIR)/utils/coder_state.c

OBJS		= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS		= $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
