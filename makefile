
# 定义变量
CC = gcc
CFLAGS = -Wall -lX11 -O0 -g3 -fsanitize=address
TARGET = easywm
SRC = main.c client.c
HEADER = client.h config.h key.h utils.h global.h

# 默认目标
all: $(TARGET)

# 目标规则
$(TARGET): $(SRC) $(HEADER)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

# 清理目标
clean:
	rm -f $(TARGET)
