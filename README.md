# SportRadar
Solution for home assignment from SportRadar.

## Assumptions and comments:
- Solution is not thread safe. It is not guaranteed to be working in multithreading environment if two threads would have to modify data.
- There is always a tradeoff between complexity of operations. Accessing (or modifying) has complexity log(n). I think it is most desirable trade-off since log(n) is relatively small number. Having some operation running in O(1) would cause that something else would have linear complexity. I think it is better to have everything log(n). Summary has O(n) complexity but there is no other choice to do that (alternatively I could return constant iterators not collection). It has as many elements as we want to return.
- I assumed some API that looks reasonable for me. I assumed that user will be able to deliver object of match structure in update. That approach can be discussed, but I decided that it is better to expect sucha behaviour from the client. Similarly I returned vector of matches instead of tuples.
- Operators such as "<" and "==" are very counterintuitive. As such they should be not used by the user. Normally I would use a wrapper class with those operators purpously deleted or written the other way. Wrapper class would have an instance of match. wrapper would be part of user API, but in my implementation I would take match from the wrapper and work only on matches as I do it now. Also wrappers would be returned in summary. I decided not to do it not to complicate this code even more. 
