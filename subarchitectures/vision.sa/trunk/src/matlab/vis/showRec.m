function showRec(ansYes,ansPy,answ,f)
%function showRec(answ,f)

global LRguiR Coma Params
global LRaxRoi LRtxFroi LRaxRec LRtxRec LRtxFrec LRtxFisRec
global currMode

numC=size(Coma.Cnames,1);
THRs=Params.THRs;
MaxTyp=1;%min(1,THRs(1)*4);

if ~isempty(answ)

  recYes=idx2name(ansYes,Coma.Cnames);
  recPy=idx2name(ansPy,Coma.Cnames);

  str=recYes;
  if ~isempty(recPy)
     str=[str '    (' recPy ' )'];
  end
  set(LRtxRec,'String',str);

   bar(LRaxRec,answ(:,2),'b');
   set(LRaxRec,'XTickLabel',Coma.Cnames(answ(:,1),:));
   axis(LRaxRec,[0 numC+1 0 MaxTyp]);
   
line([0 numC+1],[THRs(1) THRs(1)],'Color','g','Parent',LRaxRec);
line([0 numC+1],[THRs(2) THRs(2)],'Color','m','Parent',LRaxRec);
%line([0 numC+1],[THRs(3) THRs(3)],'Color','r','Parent',LRaxRec);
   
   
end

set(LRtxFisRec,'Visible','on');
set(LRtxFrec,'String',[num2str(f','%.2g  ') ' ]']);


