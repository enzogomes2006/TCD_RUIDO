"""Regera o guia de preparacao (requer reportlab, opcional pypdfium2 para QA)."""
from pathlib import Path
import re,textwrap,html
from reportlab.platypus import SimpleDocTemplate,Paragraph,Spacer,PageBreak,LongTable,TableStyle,Preformatted,KeepTogether
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet,ParagraphStyle
from reportlab.lib.enums import TA_LEFT
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.pagesizes import A4

ROOT=Path(__file__).resolve().parents[1]
FONT=Path('C:/Windows/Fonts')
if (FONT/'arial.ttf').exists():
    pdfmetrics.registerFont(TTFont('Body',str(FONT/'arial.ttf')))
    pdfmetrics.registerFont(TTFont('BodyBold',str(FONT/'arialbd.ttf')))
    pdfmetrics.registerFontFamily('Body',normal='Body',bold='BodyBold',italic='Body',boldItalic='BodyBold')
else:
    pdfmetrics.registerFontFamily('Body',normal='Helvetica',bold='Helvetica-Bold',italic='Helvetica',boldItalic='Helvetica-Bold')
    # Fallback names for portable generation.
    from reportlab.pdfbase.pdfmetrics import Font
    pdfmetrics.registerFont(Font('Body','Helvetica','WinAnsiEncoding'))
    pdfmetrics.registerFont(Font('BodyBold','Helvetica-Bold','WinAnsiEncoding'))

BLUE=colors.HexColor('#163F59');TEAL=colors.HexColor('#167B88');LIGHT=colors.HexColor('#EFF5F7')
styles=getSampleStyleSheet()
styles.add(ParagraphStyle(name='TextMain',fontName='Body',fontSize=10,leading=14,spaceAfter=8,textColor=colors.HexColor('#21303A'),allowWidows=0,allowOrphans=0))
styles.add(ParagraphStyle(name='TitleGuide',fontName='BodyBold',fontSize=25,leading=30,textColor=BLUE,spaceAfter=14))
styles.add(ParagraphStyle(name='SectionGuide',fontName='BodyBold',fontSize=17,leading=22,textColor=BLUE,spaceAfter=12))
styles.add(ParagraphStyle(name='SubGuide',fontName='BodyBold',fontSize=12,leading=16,textColor=TEAL,spaceBefore=13,spaceAfter=7,keepWithNext=True))
styles.add(ParagraphStyle(name='CellGuide',fontName='Body',fontSize=8.4,leading=11,spaceAfter=0))
styles.add(ParagraphStyle(name='CodeGuide',fontName='Courier',fontSize=8,leading=11,backColor=LIGHT,borderPadding=7,spaceBefore=6,spaceAfter=10))

def inline(s):
    s=html.escape(s)
    s=re.sub(r'\[([^]]+)\]\((https?://[^)]+)\)',r'<link href="\2" color="#167B88">\1</link>',s)
    s=re.sub(r'\*\*([^*]+)\*\*',r'<b>\1</b>',s)
    s=re.sub(r'`([^`]+)`',r'<font name="Courier">\1</font>',s)
    return s

def table(lines):
    rows=[]
    for line in lines:
        cells=[c.strip() for c in line.strip().strip('|').split('|')]
        if all(re.fullmatch(r':?-+:?',c) for c in cells):continue
        cell_style=styles['CellGuide'] if rows else ParagraphStyle(name='Header',parent=styles['CellGuide'],fontName='BodyBold',textColor=colors.white)
        rows.append([Paragraph(inline(c),cell_style) for c in cells])
    cols=len(rows[0]);width=495
    widths={4:[104,124,183,84],5:[54,90,125,106,120]}.get(cols,[width/cols]*cols)
    if 'Offset' in lines[0]:widths=[55,110,45,285]
    t=LongTable(rows,colWidths=widths,repeatRows=1,hAlign='LEFT')
    t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),BLUE),('TEXTCOLOR',(0,0),(-1,0),colors.white),
        ('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),
        ('TOPPADDING',(0,0),(-1,-1),7),('BOTTOMPADDING',(0,0),(-1,-1),7),
        ('ROWBACKGROUNDS',(0,1),(-1,-1),[colors.white,LIGHT]),('LINEBELOW',(0,0),(-1,0),.8,TEAL)]))
    return t

def markdown(text):
    result=[];lines=text.splitlines();i=0
    while i<len(lines):
        line=lines[i].strip()
        if not line:i+=1;continue
        if line.startswith('```'):
            code=[];i+=1
            while i<len(lines) and not lines[i].startswith('```'):
                code.extend(textwrap.wrap(lines[i],width=91,replace_whitespace=False,drop_whitespace=False) or ['']);i+=1
            result.append(Preformatted('\n'.join(code),styles['CodeGuide']));i+=1;continue
        if line.startswith('|'):
            group=[]
            while i<len(lines) and lines[i].strip().startswith('|'):group.append(lines[i]);i+=1
            result.extend([table(group),Spacer(1,10)]);continue
        if line.startswith('# '):result.append(Paragraph(inline(line[2:]),styles['SectionGuide']));i+=1;continue
        if line.startswith('## '):result.append(Paragraph(inline(line[3:]),styles['SubGuide']));i+=1;continue
        result.append(Paragraph(inline(line),styles['TextMain']));i+=1
    return result

def decorate(canvas,doc):
    canvas.saveState();w,h=A4
    canvas.setStrokeColor(TEAL);canvas.setLineWidth(1);canvas.line(50,h-39,w-50,h-39)
    canvas.setFont('Body',8);canvas.setFillColor(BLUE)
    canvas.drawString(50,h-29,'TCD_RUIDO | GUIA DE PREPARACAO E DEFESA')
    canvas.drawString(50,29,'Material de estudo | resultados reais devem ser produzidos pela equipe')
    canvas.drawRightString(w-50,29,str(doc.page));canvas.restoreState()

def main():
    output=ROOT/'output/pdf/Guia_Entrega_e_Defesa_Telemetria_ESP32.pdf';output.parent.mkdir(parents=True,exist_ok=True)
    story=[Spacer(1,55),Paragraph('Telemetria UART<br/>Deteccao de erros',styles['TitleGuide']),
        Paragraph('Roteiro para preparar a entrega principal e defender o trabalho',styles['SectionGuide']),Spacer(1,12),
        Paragraph('<b>Principal:</b> teoria, especificacao, exemplos manuais, demonstracao no computador e estrutura do relatorio.',styles['TextMain']),
        Paragraph('<b>Extra separado:</b> montagem e demonstracao fisica com tres ESP32.',styles['TextMain']),Spacer(1,17),
        Paragraph('Base: curso proprio de 21 dias (47 paginas) e enunciado da atividade (2 paginas), fornecidos pelo estudante.',styles['TextMain']),
        Paragraph('Grupo de tres estudantes; defesa e nota individuais. Use este guia para estudar e executar, e escreva a analise com suas palavras. Este PDF nao substitui um relatorio final com evidencias autorais.',styles['TextMain']),
        Paragraph('Repositorio: <link href="https://github.com/enzogomes2006/TCD_RUIDO" color="#167B88">github.com/enzogomes2006/TCD_RUIDO</link>',styles['TextMain']),
        Spacer(1,18),Paragraph('Como navegar',styles['SubGuide']),
        Paragraph('1. Roteiro de defesa de 20 minutos e demonstracoes teoricas.<br/>2. Especificacao proposta do protocolo.<br/>3. Estrutura editavel do relatorio.<br/>4. Matriz de requisitos e evidencias.<br/>5. Extra: bancada, piloto e coleta real.',styles['TextMain']),
        Spacer(1,12),Paragraph('Escopo: a parte fisica e extra segundo a orientacao do estudante. O PDF original pede ruido real, medicoes e demo ao vivo. A matriz registra as evidencias que dependeriam desse escopo experimental.',styles['TextMain']),
        Paragraph('Preparado em 4 de outubro de 2026 | versao inicial para revisao da equipe',styles['TextMain'])]
    for file in ('roteiro.md','especificacao.md','relatorio_modelo.md','requisitos.md','extra_bancada.md'):
        story.append(PageBreak());story.extend(markdown((ROOT/'docs'/file).read_text(encoding='utf-8')))
    story.extend([PageBreak(),Paragraph('Fontes e verificacao',styles['SectionGuide'])])
    sources=[
        'Curso Completo de 21 Dias - Telemetria UART com ESP32, fornecido pelo estudante. Consultado integralmente; roteiro vinculado aos dias 1..21.',
        'Trabalho Pratico: Deteccao de Erro em Telemetria sob Ruido Real (ESP32), enunciado fornecido, paginas 1 e 2.',
        '[UART e Arduino-ESP32 - Espressif](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/serial.html)',
        '[GPTimer - Espressif](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gptimer.html)',
        '[esp_timer - Espressif](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_timer.html)',
        'Verificacao: nove testes automatizados em Python e teste do nucleo C++ no computador. Vetor CRC F4, exemplos de residuo, colisao de dois MSBs, rajadas de ate oito bits, classificacao e sequencia.',
        'Nao verificado: compilacao dos tres sketches no pacote ESP32, funcionamento eletrico, pulsos reais, tempos TX/RX e recuperacao em placa. Nenhuma tabela real foi preenchida.',
        'O material teve apoio de IA. A regra do enunciado sobre trabalhos baseados em IA precisa ser observada pela equipe: o relatorio final, as contribuicoes e a defesa precisam refletir o aprendizado e a execucao dos estudantes.'
    ]
    for s in sources:story.append(Paragraph(inline(s),styles['TextMain']))
    doc=SimpleDocTemplate(str(output),pagesize=A4,rightMargin=50,leftMargin=50,topMargin=56,bottomMargin=48,
        title='Guia de entrega e defesa - Telemetria UART ESP32',author='Material de apoio para a equipe TCD_RUIDO')
    doc.build(story,onFirstPage=decorate,onLaterPages=decorate)
    print(output)

if __name__=='__main__':main()
