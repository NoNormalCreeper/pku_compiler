// cat /opt/bin/testcases/lv7/08_if_break.c
int main()
{
    int i = 0;
    while (i < 10) {
        if (i == 5) {
            break;
        }
        i = i + 1;
    }
    return i;
}