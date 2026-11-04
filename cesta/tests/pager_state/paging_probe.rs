pub fn probe_retired(s:&MappingLedger)->[Option<Binding>;PAGES]{s.retired}
pub fn probe_closed(s:&MappingLedger)->[(u64,Option<CleanupRecord>);CLIENTS]{
 core::array::from_fn(|i|(s.retired_highwater[i],s.closed[i]))
}
