program parstest1;

const
    a = 1;
    b = 1.1;
    c = 'abc';
    // f = a;
    // e = f;

var
    i : integer;
    p : array[2] of integer;


procedure empty();
begin
end;


begin
    i := (i + 1) / 2 * 3 mod 4;
    if a then
        i := 1;
    elsif a then
        i := b;
    else
        i := i;
    end
    empty();
end.
