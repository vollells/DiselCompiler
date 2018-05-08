program opttest1;
const
  FOO = 2;
var
  i : integer;
  a : integer;
  p : real;
  q : real;

function f(l: integer) : integer;
begin
  return l;
end;

procedure proc(l: integer);
begin
  i := l;
end;

begin
  i := FOO + 2;
  i := 3 * 1;
  i := - ( 3 * 1 );
  i := i - ( 3 * 1 );
  i := i or ( 3 * 1 );
  i := ( i + 1 ) + ( i + (- ( 63 + 3 ) ));
  i := ( i + 1 ) or ( i + (- ( 64 + 3 ) ));
  i := f( 3 * 1 );
  a := i + 1;
  proc( 3 * 1 );
  if(2 - 3 < 4) then
    i := a + 5;
  end;
  if(5 AND 2) then
    p := 15.0 / 2;
    q := 5.6 + 1.0;
  elsif (3 or 1) then
    q := 6.6 + 1.0;
  end;
end.
