// make && ./diesel -y -q ../testpgm/simple.d && cat d.out && ./a.out
program simple;

var
e: real;

//procedure A(b: integer; c: integer; d: integer);

    //procedure Q;
    //    procedure W;
    //    begin;
    //        b := b + 1;
    //        c := c + 1;
    //        d := d + 1;
    //    end;
    //begin;
    //    W();
    //end;

//begin;
//    WRITE(b);
//    Q();
//    WRITE(b);
//    WRITE(10);
//end;

begin;
    e := 2.0 + 2.0;
    if e > 2.0 then
       WRITE(65);
     else
       WRITE(66);
     end;
    //A(65, 0, 0);
end.
