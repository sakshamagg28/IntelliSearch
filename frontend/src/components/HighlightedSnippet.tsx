interface HighlightedSnippetProps {
  snippet: string;
}

export function HighlightedSnippet({ snippet }: HighlightedSnippetProps) {
  const parts = snippet.split(/(\[[^\]]+\])/g).filter(Boolean);

  return (
    <p className="text-sm leading-6 text-slate-700">
      {parts.map((part, index) => {
        const highlighted = part.startsWith('[') && part.endsWith(']');
        const text = highlighted ? part.slice(1, -1) : part;

        return highlighted ? (
          <mark key={`${part}-${index}`} className="rounded bg-yellow-100 px-1 text-slate-950">
            {text}
          </mark>
        ) : (
          <span key={`${part}-${index}`}>{text}</span>
        );
      })}
    </p>
  );
}
