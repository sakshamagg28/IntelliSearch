import type { RankingDetails } from '../types/api';

interface ExplainPanelProps {
  details: RankingDetails;
}

const scoreRows: Array<[keyof RankingDetails, string, string]> = [
  ['bm25_score', 'BM25', 'bg-teal-500'],
  ['fuzzy_score', 'Fuzzy', 'bg-indigo-500'],
  ['expansion_score', 'Expansion', 'bg-amber-500'],
  ['phrase_boost', 'Phrase', 'bg-rose-500'],
];

export function ExplainPanel({ details }: ExplainPanelProps) {
  const maxScore = Math.max(
    ...scoreRows.map(([key]) => Math.abs(Number(details[key]) || 0)),
    1,
  );

  return (
    <div className="mt-4 rounded-lg border border-slate-200 bg-slate-50 p-4">
      <div className="grid gap-3 md:grid-cols-2">
        {scoreRows.map(([key, label, color]) => {
          const value = Number(details[key]) || 0;
          const width = `${Math.max(6, (Math.abs(value) / maxScore) * 100)}%`;

          return (
            <div key={key}>
              <div className="mb-1 flex items-center justify-between text-xs font-medium text-slate-600">
                <span>{label}</span>
                <span>{value.toFixed(5)}</span>
              </div>
              <div className="h-2 rounded-full bg-white">
                <div className={`h-2 rounded-full ${color}`} style={{ width }} />
              </div>
            </div>
          );
        })}
      </div>

      <div className="mt-4 grid gap-3 text-xs text-slate-600 md:grid-cols-3">
        <TokenGroup label="Matched" values={details.matched_terms} />
        <TokenGroup label="Expansions" values={details.expansions} />
        <TokenGroup label="Fuzzy" values={details.fuzzy_matches} />
      </div>
    </div>
  );
}

function TokenGroup({ label, values }: { label: string; values: string[] }) {
  return (
    <div>
      <p className="mb-2 font-semibold text-slate-700">{label}</p>
      {values.length > 0 ? (
        <div className="flex flex-wrap gap-1.5">
          {values.map((value) => (
            <span key={value} className="rounded-md border border-slate-200 bg-white px-2 py-1">
              {value}
            </span>
          ))}
        </div>
      ) : (
        <span className="text-slate-400">None</span>
      )}
    </div>
  );
}
