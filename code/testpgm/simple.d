// make && ./diesel -y -q ../testpgm/simple.d && cat d.out && ./a.out
program simple;

procedure A(b: integer);
begin;
    WRITE(b);
    b := b+1;
    WRITE(b);
    WRITE(10);
end;

begin;
    A(77);
end.
