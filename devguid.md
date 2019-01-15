# Developer guid
## Valid keywords 
as
asm
assert
async
awit
bool
break
catch
char
class
const
const
continue
cte
cte?
delete
double
else
enum
explicit
export
extern
false
float
fn
for
from
if
import
in
inline
int
match
mod
namespace
new
op
orderby
priv
pub
return
select
self
sizeof
static
struct
this
throw
trait
true
try
typedef
typeid
typename
typeof
uint
unsigned
using
void
where
while
yield


## Statements
### Function Declaration
```rust
inline [cte/cte?] static [pub/priv] export extern fn snake_case_name<T=float /*default generic*/,U: Atomic + Add>(a /*infered*/, b : T/*simple generic*/,c : U/*must implement Atomic and Add*/,d : X /*inherited from a class/struct*/,e : Atomic /*must implement atomic*/,f : i32 /*standart old style*/,g .../*varadic params of any type,must be last arg*/ /* could have type like g... : T*/ /* can be defaulted like g... = [0]*/) -> typeof(U+i32)/*can be left out as it is infered*/ assert [f>0] 
  return c+f;
```

### Lambda Functions
```rust
[cte/cte?] inline |a,b| return a+b (1,3)
```

### Class Declaration
```rust
class CamelCaseName<X> : BaseClass,Trait,AbstractClass
```

### Enum Declaration
```rust
enum CameCaseName<T> : size_type
  TypedArg(arg:T),
  NonType,
  ManyTypes(first_args : i32,second_arg : float,char/*arg name isn't neceserry)
```
