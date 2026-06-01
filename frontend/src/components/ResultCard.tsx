import { FileText } from 'lucide-react';

import type { SearchResult } from '../types/api';
import { ExplainPanel } from './ExplainPanel';
import { HighlightedSnippet } from './HighlightedSnippet';

interface ResultCardProps {
  result: SearchResult;
  explain: boolean;
}

export function ResultCard({ result, explain }: ResultCardProps) {
  return (
    <article className="rounded-lg border border-slate-200 bg-white p-5 shadow-sm">
      <div className="flex flex-col gap-3 sm:flex-row sm:items-start sm:justify-between">
        <div className="flex min-w-0 items-start gap-3">
          <div className="grid h-10 w-10 shrink-0 place-items-center rounded-lg bg-slate-100 text-slate-600">
            <FileText className="h-5 w-5" aria-hidden="true" />
          </div>
          <div className="min-w-0">
            <div className="flex flex-wrap items-center gap-2">
              <span className="rounded-md bg-teal-50 px-2 py-1 text-xs font-semibold text-teal-700">
                Rank {result.rank}
              </span>
              <span className="text-sm font-medium text-slate-700">Doc {result.doc_id}</span>
            </div>
            <div className="mt-3">
              <HighlightedSnippet snippet={result.snippet} />
            </div>
          </div>
        </div>
        <div className="rounded-lg border border-slate-200 bg-slate-50 px-3 py-2 text-right">
          <p className="text-xs font-medium uppercase text-slate-500">Score</p>
          <p className="font-mono text-lg font-semibold text-slate-950">
            {result.score.toFixed(5)}
          </p>
        </div>
      </div>

      {explain && result.ranking_details ? <ExplainPanel details={result.ranking_details} /> : null}
    </article>
  );
}
