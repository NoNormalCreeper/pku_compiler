// cat /opt/bin/testcases/lv7/09_continue.c
int main()
{
    int i = 0;
    while (i < 10) {
        i = 20;
        continue;
        i = i + 1;
    }
    return i;
}