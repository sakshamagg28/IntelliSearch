import { CheckCircle2, CircleAlert } from 'lucide-react';
import { useEffect, useState } from 'react';

import { api } from '../services/api';

export function HealthBadge() {
  const [status, setStatus] = useState<'loading' | 'ok' | 'degraded' | 'offline'>('loading');

  useEffect(() => {
    let mounted = true;
    api
      .health()
      .then((health) => {
        if (mounted) {
          setStatus(health.status === 'ok' ? 'ok' : 'degraded');
        }
      })
      .catch(() => {
        if (mounted) {
          setStatus('offline');
        }
      });

    return () => {
      mounted = false;
    };
  }, []);

  const isOk = status === 'ok';
  const Icon = isOk ? CheckCircle2 : CircleAlert;

  return (
    <div
      className={[
        'flex min-h-9 items-center gap-2 rounded-lg border px-3 text-sm font-medium',
        isOk
          ? 'border-emerald-200 bg-emerald-50 text-emerald-700'
          : 'border-amber-200 bg-amber-50 text-amber-700',
      ].join(' ')}
    >
      <Icon className="h-4 w-4" aria-hidden="true" />
      {status === 'loading' ? 'Checking' : status}
    </div>
  );
}
