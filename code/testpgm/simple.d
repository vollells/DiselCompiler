// make && ./diesel -y -q ../testpgm/simple.d && cat d.out && ./a.out
program simple;

procedure A(b: integer);
          procedure C;
          begin;
            b := b+1;
          end;

begin;
    WRITE(b);
    C();
    WRITE(b);
    WRITE(10);
end;

begin;
    A(65);
end.
