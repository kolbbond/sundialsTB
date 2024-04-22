% 230222 - modified for debug
function new_mondata = idm_monitor(call, t, yy, yQ, Ns, yyS, fct, mondata)

%
% Wrapper around the actual user-provided Matlab function
%
global debug_clf;

N = length(yy);
yyS = reshape(yyS, N, Ns);

if isempty(mondata)
  feval(fct, call, t, yy, yQ, yyS);
  new_mondata = [];
else
  if(debug_clf)
    %disp('in idm_monitor'); disp('plot mondata'); disp(mondata); 
  end
  
  new_mondata = feval(fct, call, t, yy, yQ, yyS, mondata);
end

